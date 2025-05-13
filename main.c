

// ----- INCLUDES DO PROJETO -----
#include "include/config.h"
#include "include/display.h"
// #include "lwipopts.h" // Não incluir aqui
#define REQUEST_BUFFER_SIZE 2048

// Credenciais WIFI
#define WIFI_SSID     "CNAnet_ADRIANA"
#define WIFI_PASSWORD "vidanova"

// ----- GLOBAIS -----
static ssd1306_t ssd_global;
static char pico_ip_address[20] = "N/A";

// Variáveis Globais para Simulação
static float g_temperatura_ar_simulada = 25.0f;
static float g_umidade_ar_simulada = 60.0f;
static bool g_luminosidade_claro = true;
static bool g_estado_rele_irrigacao = false;
static bool g_estado_rele_luz = false;

typedef enum { RESERVATORIO_ALTO, RESERVATORIO_MEDIO, RESERVATORIO_BAIXO } nivel_reservatorio_t;
static nivel_reservatorio_t g_nivel_reservatorio_simulado_enum = RESERVATORIO_ALTO;

// ----- PROTÓTIPOS -----
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
float temp_read(void);
void user_request(char **request_str_ptr); // Assumindo que atualiza as globais g_estado_rele...

// ----- FUNÇÃO PRINCIPAL (main) -----
int main() {
    stdio_init_all();
    sleep_ms(500);

    // ----- INICIALIZAÇÃO DE PERIFÉRICOS -----
    printf("Inicializando perifericos...\n");
    buttons_init(); // Configura BUTTON_A_PIN e BUTTON_B_PIN
    joystick_init();
    // rgb_led_init();    // Inicialize se usar LED RGB para status geral
    led_matrix_init();
    display_init(&ssd_global);

    // update_rgb_led_by_status(RGB_STATUS_OFF); // Se usar RGB de status
    display_startup_screen(&ssd_global);
    display_message(&ssd_global, "Sistema OK", "Iniciando WiFi...");

    // ----- CONEXÃO WI-FI -----
    printf("WiFi: Inicializando cyw43...\n");
    // update_rgb_led_by_status(RGB_STATUS_CONNECTING_WIFI); // Se usar RGB de status

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL)) {
        printf("ERRO FATAL: cyw43_arch_init falhou\n");
        // update_rgb_led_by_status(RGB_STATUS_ERROR_WIFI_SYSTEM);
        display_message(&ssd_global, "ERRO FATAL", "WiFi Init Falhou");
        return -1;
    }
    printf("WiFi: cyw43 inicializado OK.\n");
    cyw43_arch_enable_sta_mode();
    cyw43_arch_gpio_put(LED_PIN, 0); // LED Onboard do Pico W apagado

    printf("WiFi: Conectando a '%s'...\n", WIFI_SSID);
    display_message(&ssd_global, "Conectando:", WIFI_SSID);
    cyw43_arch_gpio_put(LED_PIN, 1); // LED Onboard LIGADO durante tentativa

    // Conexão Síncrona Simples (como no código original do professor)
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("ERRO: Falha ao conectar ao Wi-Fi.\n");
        // update_rgb_led_by_status(RGB_STATUS_ERROR_WIFI_SYSTEM);
        display_message(&ssd_global, "WiFi ERRO", "Falha Conexao");
        cyw43_arch_gpio_put(LED_PIN, 1); // LED Onboard permanece LIGADO
        return -1;
    }

    printf("WiFi: Conectado com sucesso!\n");
    // update_rgb_led_by_status(RGB_STATUS_SERVER_RUNNING); // Mover para após TCP OK
    cyw43_arch_gpio_put(LED_PIN, 0); // LED Onboard DESLIGADO

    if (netif_default) {
        strncpy(pico_ip_address, ipaddr_ntoa(netif_ip4_addr(netif_default)), sizeof(pico_ip_address) - 1);
        pico_ip_address[sizeof(pico_ip_address) - 1] = '\0';
        printf("IP do dispositivo: %s\n", pico_ip_address);
        display_message(&ssd_global, "WiFi OK!", pico_ip_address);
    } else {
        printf("WiFi Conectado, mas sem info de IP.\n");
        display_message(&ssd_global, "WiFi OK!", "Sem IP Addr");
    }
    sleep_ms(1000);

    // ----- CONFIGURAÇÃO DO SERVIDOR TCP -----
    printf("TCP Server: Configurando...\n");
    display_message(&ssd_global, pico_ip_address, "Iniciando Serv");
    struct tcp_pcb *server = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!server) {
        printf("ERRO: tcp_new() falhou.\n");
        // update_rgb_led_by_status(RGB_STATUS_ERROR_WIFI_SYSTEM);
        display_message(&ssd_global, pico_ip_address, "ERRO TCP New");
        return -1;
    }
    err_t bind_err = tcp_bind(server, IP_ADDR_ANY, 80);
    if (bind_err != ERR_OK) {
        printf("ERRO: tcp_bind() falhou (%d).\n", bind_err);
        // update_rgb_led_by_status(RGB_STATUS_ERROR_WIFI_SYSTEM);
        display_message(&ssd_global, pico_ip_address, "ERRO TCP Bind");
        tcp_close(server);
        return -1;
    }
    server = tcp_listen_with_backlog(server, 1); // Backlog baixo para simplificar e economizar memória
    if (!server) {
         printf("ERRO: tcp_listen() falhou.\n");
         // update_rgb_led_by_status(RGB_STATUS_ERROR_WIFI_SYSTEM);
         display_message(&ssd_global, pico_ip_address, "ERRO TCP Listen");
         return -1;
    }
    tcp_accept(server, tcp_server_accept);
    printf("TCP Server: Ouvindo na porta 80.\n");
    // update_rgb_led_by_status(RGB_STATUS_SERVER_RUNNING); // LED RGB VERDE
    display_message(&ssd_global, pico_ip_address, "Servidor OK");

    adc_set_temp_sensor_enabled(true); // Sensor temp interno

    // ----- LOOP PRINCIPAL -----
    printf("Entrando no loop principal.\n");
    uint32_t last_oled_update_time = 0;

    while (true) {
        cyw43_arch_poll(); 

        uint16_t adc_x_raw = read_adc(JOYSTICK_X_ADC_CHANNEL);
        uint16_t adc_y_raw = read_adc(JOYSTICK_Y_ADC_CHANNEL);

        g_umidade_ar_simulada = 30.0f + (adc_x_raw / 4095.0f) * 60.0f;
        g_temperatura_ar_simulada = 10.0f + (adc_y_raw / 4095.0f) * 30.0f;

        if (button_a_pressed()) {
            g_luminosidade_claro = !g_luminosidade_claro;
            printf("Luminosidade Sim.: %s\n", g_luminosidade_claro ? "CLARO" : "ESCURO");
        }

        if (button_b_pressed()) { // Assume BUTTON_B_PIN configurado para reservatório
            switch (g_nivel_reservatorio_simulado_enum) {
                case RESERVATORIO_ALTO:  g_nivel_reservatorio_simulado_enum = RESERVATORIO_MEDIO; printf("Reserv.: MEDIO\n"); break;
                case RESERVATORIO_MEDIO: g_nivel_reservatorio_simulado_enum = RESERVATORIO_BAIXO; printf("Reserv.: BAIXO\n"); break;
                case RESERVATORIO_BAIXO: g_nivel_reservatorio_simulado_enum = RESERVATORIO_ALTO;  printf("Reserv.: ALTO\n"); break;
            }
        }

        // Matriz de LEDs é atualizada DIRETAMENTE por user_request agora

        // Atualiza OLED periodicamente (exemplo)
        if (time_us_32() - last_oled_update_time > 2000000) { // A cada 2 segundos
            char line1_buf[21]; // Buffer para linha 1 (16 chars + margem + nulo)
            char line2_buf[21]; // Buffer para linha 2
            char line3_buf[21]; // Buffer para linha 3 (se sua função display_message suportar)
            
            snprintf(line1_buf, sizeof(line1_buf), "T:%.1fC U:%.1f%%", g_temperatura_ar_simulada, g_umidade_ar_simulada);
            const char* lum_str = g_luminosidade_claro ? "Luz:CLA" : "Luz:ESC";
            const char* res_str;
            switch(g_nivel_reservatorio_simulado_enum){
                case RESERVATORIO_ALTO: res_str = "Res:ALTO"; break;
                case RESERVATORIO_MEDIO: res_str = "Res:MED"; break;
                default: res_str = "Res:BAIXO"; break;
            }
            snprintf(line2_buf, sizeof(line2_buf), "%s %s", lum_str, res_str);
            // Para uma terceira linha, você precisaria de display_message com 3 args
            // snprintf(line3_buf, sizeof(line3_buf), "IR:%s L:%s", g_estado_rele_irrigacao?"ON":"OFF", g_estado_rele_luz?"ON":"OFF");
            // display_message_3lines(&ssd_global, line1_buf, line2_buf, line3_buf);
            display_message(&ssd_global, line1_buf, line2_buf); // Usando sua display_message atual
            last_oled_update_time = time_us_32();
        }


        sleep_ms(50); // Reduzido para polls mais frequentes e resposta mais rápida
    }
    // Inatingível
    cyw43_arch_deinit();
    // return 0;
}

// ----- IMPLEMENTAÇÕES DAS FUNÇÕES -----
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK || !newpcb) { printf("Accept Erro %d\n", err); return ERR_VAL; }
    printf("Nova conexao TCP aceita do IP: %s\n", ipaddr_ntoa(&newpcb->remote_ip));
    tcp_arg(newpcb, NULL);
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// user_request agora atualiza as globais de estado dos relés e a matriz diretamente
void user_request(char **request_str_ptr) {
    if (!request_str_ptr || !(*request_str_ptr)) return;
    char *req = *request_str_ptr;
    printf("USER_REQUEST: Recebido '%s'\n", req);

    if (strstr(req, "GET /irrigacao_on")) {
        led_matrix_draw_water_drop();
        g_estado_rele_irrigacao = true;
        printf("USER_REQUEST: Irrigacao LIGADA (Matriz: Gota)\n");
    } else if (strstr(req, "GET /irrigacao_off")) {
        g_estado_rele_irrigacao = false;
        if (!g_estado_rele_luz) led_matrix_clear(); else led_matrix_draw_light_icon();
        printf("USER_REQUEST: Irrigacao DESLIGADA\n");
    } else if (strstr(req, "GET /luz_on")) {
        led_matrix_draw_light_icon();
        g_estado_rele_luz = true;
        printf("USER_REQUEST: Luz LIGADA (Matriz: Luz)\n");
    } else if (strstr(req, "GET /luz_off")) {
        g_estado_rele_luz = false;
        if (!g_estado_rele_irrigacao) led_matrix_clear(); else led_matrix_draw_water_drop();
        printf("USER_REQUEST: Luz DESLIGADA\n");
    }
}

float temp_read(void) { /* ... como antes ... */ }

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (err != ERR_OK && err != ERR_ABRT) {
        printf("Recv Erro %d\n", err);
        if (p) pbuf_free(p);
        tcp_close(tpcb); return err;
    }
    if (!p) {
        printf("Recv: Conexao fechada pelo cliente.\n");
        tcp_close(tpcb); return ERR_OK;
    }
    tcp_recved(tpcb, p->tot_len);

    char request_buffer[REQUEST_BUFFER_SIZE];
    uint16_t copied_len = pbuf_copy_partial(p, request_buffer, sizeof(request_buffer) - 1, 0);
    request_buffer[copied_len] = '\0';
    pbuf_free(p);

    char *request_ptr_local = request_buffer;
    user_request(&request_ptr_local);

    const char* luminosidade_atual_str = g_luminosidade_claro ? "CLARO" : "ESCURO";
    const char* reservatorio_nivel_str_html;
    switch (g_nivel_reservatorio_simulado_enum) {
        case RESERVATORIO_ALTO:  reservatorio_nivel_str_html = "ALTO";  break;
        case RESERVATORIO_MEDIO: reservatorio_nivel_str_html = "MEDIO"; break;
        case RESERVATORIO_BAIXO: reservatorio_nivel_str_html = "BAIXO"; break;
        default:                 reservatorio_nivel_str_html = "N/D";   break;
    }
    const char* irrigacao_atual_str = g_estado_rele_irrigacao ? "LIGADO" : "DESLIGADO";
    const char* luz_atual_str = g_estado_rele_luz ? "LIGADO" : "DESLIGADO";

    char html[4096]; // Buffer grande para o HTML
    snprintf(html, sizeof(html),
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
            "<!DOCTYPE html><html lang=\"pt-BR\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" // META REFRESH REMOVIDO
            "<title>Estação Agro PicoW - JS Update</title>"
            "<style>body{font-family:Arial,sans-serif;margin:0;padding:20px;background-color:#eef3e8;color:#333}.container{max-width:800px;margin:20px auto;background-color:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}header{text-align:center;margin-bottom:25px;border-bottom:2px solid #4caf50;padding-bottom:15px}header h1{color:#38761d;font-size:2.2em;margin:0}.grid-container{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:15px;margin-bottom:20px}.sensor-card{background-color:#f9f9f9;padding:15px;border-radius:6px;border-left:5px solid #66bb6a; text-align:center;}.sensor-card h2{font-size:1.1em;color:#1b5e20;margin-top:0;margin-bottom:8px}.sensor-card p{margin:5px 0;font-size:0.95em}.value{font-weight:bold;font-size:1.2em;color:#2e7d32}.unit{font-size:0.8em;color:#555}button{font-size:0.8em;padding:6px 10px;margin:5px 2px; border-radius:4px; border:1px solid #ccc; background-color:#f0f0f0; cursor:pointer; min-width:50px;} button:hover{background-color:#e0e0e0;}footer{text-align:center;margin-top:30px;font-size:0.9em;color:#777}</style>"
            "</head><body><div class=\"container\"><header><h1>Monitor Agrícola Inteligente</h1><p style=\"font-size:0.9em; color:#555;\">Fase 1: Simulação JS</p></header>"
            "<h2>Condições Atuais (Simuladas)</h2><div class=\"grid-container\">"
            "<div class=\"sensor-card\"><h2>Temperatura Ar</h2><p><span class=\"value\" id=\"temp-ar\">%.1f</span><span class=\"unit\">°C</span></p></div>" // ID adicionado
            "<div class=\"sensor-card\"><h2>Umidade Ar</h2><p><span class=\"value\" id=\"umid-ar\">%.1f</span><span class=\"unit\">%%</span></p></div>" // ID adicionado
            "<div class=\"sensor-card\"><h2>Luminosidade</h2><p><span class=\"value\" id=\"luminosidade\">%s</span></p></div>"         // ID adicionado
            "<div class=\"sensor-card\"><h2>Reservatório</h2><p><span class=\"value\" id=\"reservatorio\">%s</span></p></div></div>"   // ID adicionado
            "<h2>Controle de Atuadores (Simulados)</h2><div class=\"grid-container\">"
            "<div class=\"sensor-card\"><h2>Irrigação</h2><p>Status: <span class=\"value\" id=\"status-irrigacao\">%s</span></p>" // ID adicionado
            "<form action=\"/irrigacao_on\" method=\"get\" style=\"display:inline;\"><button type=\"submit\">LIGAR</button></form>" // Adicionado type="submit"
            "<form action=\"/irrigacao_off\" method=\"get\" style=\"display:inline;\"><button type=\"submit\">DESLIGAR</button></form>" // Adicionado type="submit"
            "</div>"
            "<div class=\"sensor-card\"><h2>Luz Artificial</h2><p>Status: <span class=\"value\" id=\"status-luz\">%s</span></p>" // ID adicionado
            "<form action=\"/luz_on\" method=\"get\" style=\"display:inline;\"><button type=\"submit\">LIGAR</button></form>"         // Adicionado type="submit"
            "<form action=\"/luz_off\" method=\"get\" style=\"display:inline;\"><button type=\"submit\">DESLIGAR</button></form>"       // Adicionado type="submit"
            "</div>"
            "</div>"
            // "<footer><p><i>PicoW WebServer v1.0 - Atualiza a cada 5 seg.</i></p></footer>" // Rodapé removido para simplicidade dos placeholders no JS
            "<footer><p id=\"footer-info\"><i>PicoW WebServer v1.0 - JS Update</i></p></footer>"
            "</div>" // Fechamento do .container

            "<script>"
            "function updateValues() {"
            "  fetch(window.location.pathname) " // Busca a URL atual (a própria página)
            "    .then(response => response.text())"
            "    .then(html => {"
            "      const parser = new DOMParser();"
            "      const doc = parser.parseFromString(html, 'text/html');"
            "      const newTempAr = doc.getElementById('temp-ar')?.textContent;" // Use ? para safe navigation
            "      const newUmidAr = doc.getElementById('umid-ar')?.textContent;"
            "      const newLuminosidade = doc.getElementById('luminosidade')?.textContent;"
            "      const newReservatorio = doc.getElementById('reservatorio')?.textContent;"
            "      const newStatusIrrigacao = doc.getElementById('status-irrigacao')?.textContent;"
            "      const newStatusLuz = doc.getElementById('status-luz')?.textContent;"

            "      if (newTempAr) document.getElementById('temp-ar').textContent = newTempAr;"
            "      if (newUmidAr) document.getElementById('umid-ar').textContent = newUmidAr;"
            "      if (newLuminosidade) document.getElementById('luminosidade').textContent = newLuminosidade;"
            "      if (newReservatorio) document.getElementById('reservatorio').textContent = newReservatorio;"
            "      if (newStatusIrrigacao) document.getElementById('status-irrigacao').textContent = newStatusIrrigacao;"
            "      if (newStatusLuz) document.getElementById('status-luz').textContent = newStatusLuz;"
            "      console.log('Valores atualizados via JS Fetch');"
            "    })"
            "    .catch(err => console.error('Erro ao buscar dados:', err));"
            "}"
            "setInterval(updateValues, 2000);" // Atualiza a cada 2000ms (2 segundos)
            "window.onload = updateValues;" // Atualiza assim que a página carrega também
            "</script>"
            "</body></html>", // Fechamentos corretos
            g_temperatura_ar_simulada,
            g_umidade_ar_simulada,
            luminosidade_atual_str,
            reservatorio_nivel_str_html,
            irrigacao_atual_str,
            luz_atual_str  
            );

    err_t write_err = tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
    if (write_err == ERR_OK) {
        tcp_output(tpcb);
    } else {
        printf("Erro %d ao escrever HTML para o buffer TCP.\n", write_err);
        tcp_close(tpcb);
        // Não precisa 'return write_err;' aqui, LwIP gerencia. Deixar return ERR_OK no final da função é melhor.
    }
    return ERR_OK;
}