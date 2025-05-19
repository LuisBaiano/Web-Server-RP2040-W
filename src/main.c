
// ----- INCLUDES DO PROJETO -----
#include "include/config.h"
#include "include/display.h"
#include "include/external/dht22.h"
#include "include/external/ldr.h"
#define REQUESTFER_SIZE 2048


// Credenciais WIFI
#define WIFI_SSID     "CNAnet_ADRIANA"
#define WIFI_PASSWORD "vidanova"

// ----- GLOBAIS -----
static ssd1306_t ssd_global;
static char pico_ip_address[20] = "N/A";
static uint32_t last_dht_read_time = 0;

// Variáveis Globais para Simulação

static float temp_ar = 0.0f;
static float umid_ar = 0.0f;
static float luminosidade = 0.0f;
static float reservatorio = 0.0f;
static bool rele_irrigacao = false;
static bool rele_luz = false;

// ----- PROTÓTIPOS -----
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void user_request(char **request_str_ptr);

// ----- FUNÇÃO PRINCIPAL (main) -----
int main() {
    stdio_init_all();
    sleep_ms(500);
    adc_init();

    // ----- INICIALIZAÇÃO DE PERIFÉRICOS -----
    printf("Inicializando perifericos...\n");
    dht22_init_sensor();
    ldr_init_sensor();
    joystick_init();
    rgb_led_init();
    led_matrix_init();
    led_matrix_clear();
    display_init(&ssd_global);

    display_startup_screen(&ssd_global);
    display_message(&ssd_global, "Sistema OK", "Iniciando WiFi...");

    // ----- CONEXÃO WI-FI -----
    printf("WiFi: Inicializando cyw43...\n");
    rgb_led_set(RGB_CONNECTING);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL)) {
        printf("ERRO FATAL: cyw43_arch_init falhou\n");
        display_message(&ssd_global, "ERRO FATAL", "WiFi Init Falhou");
        rgb_led_set(RGB_ERROR);
        return -1;
    }
    printf("WiFi: cyw43 inicializado OK.\n");
    cyw43_arch_enable_sta_mode();
    cyw43_arch_gpio_put(LED_PIN, 0); // LED Onboard do Pico W apagado

    printf("WiFi: Conectando a '%s'...\n", WIFI_SSID);
    display_message(&ssd_global, "Conectando:", WIFI_SSID);
    cyw43_arch_gpio_put(LED_PIN, 1); // LED Onboard LIGADO durante tentativa

    // Conexão Síncrona Simples
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("ERRO: Falha ao conectar ao Wi-Fi.\n");
        rgb_led_set(RGB_ERROR); 
        display_message(&ssd_global, "WiFi ERRO", "Falha Conexao");
        cyw43_arch_gpio_put(LED_PIN, 1); // LED Onboard permanece LIGADO
        return -1;
    }

    printf("WiFi: Conectado com sucesso!\n");
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
        rgb_led_set(RGB_ERROR);
        display_message(&ssd_global, pico_ip_address, "ERRO TCP New");
        return -1;
    }
    err_t bind_err = tcp_bind(server, IP_ADDR_ANY, 80);
    if (bind_err != ERR_OK) {
        printf("ERRO: tcp_bind() falhou (%d).\n", bind_err);
        rgb_led_set(RGB_ERROR);
        display_message(&ssd_global, pico_ip_address, "ERRO TCP Bind");
        tcp_close(server);
        return -1;
    }
    server = tcp_listen_with_backlog(server, 1);
    if (!server) {
        printf("ERRO: tcp_listen() falhou.\n");
        rgb_led_set(RGB_ERROR);
        display_message(&ssd_global, pico_ip_address, "ERRO TCP Listen");
        return -1;
    }
    tcp_accept(server, tcp_server_accept);
    printf("TCP Server: Ouvindo na porta 80.\n");
    rgb_led_set(RGB_SERVER_OK);
    display_message(&ssd_global, pico_ip_address, "Servidor OK");

    // ----- LOOP PRINCIPAL -----
    printf("Entrando no loop principal.\n");
    uint32_t last_oled_update_time = 0;
    uint32_t last_ldr_read_time = 0;
    uint32_t last_joy_read_time = 0;
    

    while (true) {
        cyw43_arch_poll(); 

        uint16_t adc_x = read_adc(JOYSTICK_X_ADC_CHANNEL);
        uint16_t adc_y = read_adc(JOYSTICK_Y_ADC_CHANNEL);

        if (time_us_32() - last_ldr_read_time > (LDR_READ_TIME_MS * 1000)) {
            luminosidade = ldr_read_percentage();
        }

        //leitura continua do dht22 para obter a temperatura e a umidade
        if (time_us_32() - last_dht_read_time > (DHT_READ_TIME_MS * 1000)) {
            float temp_dht, hum_dht;
            printf("Lendo DHT22...\n");
            if (dht_read_data(&temp_dht, &hum_dht)) {
                temp_ar = temp_dht; // Atualiza globais com dados reais
                umid_ar = hum_dht;
                printf("DHT22 Leitura OK: Temperatura = %.1f C, Umidade = %.1f%% \n", temp_ar, umid_ar);
            } else {
                printf("Falha ao ler DHT22.\n");

            }
            last_dht_read_time = time_us_32();
        }

        if (time_us_32() - last_joy_read_time > (JOY_READ_TIME_MS * 1000)) {
            reservatorio = joytisck_read_percentage();
        }


        // Atualiza OLED periodicamente
        if (time_us_32() - last_oled_update_time > 500000) {
            ssd1306_fill(&ssd_global, false);

            uint8_t x = 0, y = 0, w = 128, h = 60; 
            uint8_t col_x = x + 65; 
            uint8_t row_h = 12;

            ssd1306_rect(&ssd_global, y, x, w, h, true, false);

            // Linha divisória horizontal (cabeçalho)
            for (int i = 1; i < 5; i++) {
                ssd1306_hline(&ssd_global, x, x + w, y + i * row_h, true);
            }
            // Linha vertical entre colunas
            ssd1306_vline(&ssd_global, col_x, y, y + h, true);
            // Cabeçalhos
            ssd1306_draw_string(&ssd_global, "Param", x + 2, y + 2);
            ssd1306_draw_string(&ssd_global, "Valor", col_x + 5, y + 2);
            // Linha 1
            ssd1306_draw_string(&ssd_global, "Temp.", x + 2, y + 2 + row_h);
            char temp_str[12];
            snprintf(temp_str, sizeof(temp_str), "%.1f C", temp_ar);
            ssd1306_draw_string(&ssd_global, temp_str, col_x + 5, y + 2 + row_h);
            // Linha 2
            ssd1306_draw_string(&ssd_global, "Umidade", x + 2, y + 2 + row_h * 2);
            char umid_str[12];
            snprintf(umid_str, sizeof(umid_str), "%.1f %%", umid_ar);
            ssd1306_draw_string(&ssd_global, umid_str, col_x + 5, y + 2 + row_h * 2);
            // Linha 3
            ssd1306_draw_string(&ssd_global, "Luz", x + 2, y + 2 + row_h * 3);
            const char* lum_str;
            if (luminosidade > 66.0f) lum_str = "ALTA";
            else if (luminosidade > 33.0f) lum_str = "MEDIA";
            else lum_str = "ESCURO";
            ssd1306_draw_string(&ssd_global, lum_str, col_x + 5, y + 2 + row_h * 3);
            // Linha 4
            ssd1306_draw_string(&ssd_global, "Reserv.", x + 2, y + 2 + row_h * 4);
            const char* res_str;
            if (reservatorio > 66.0f) res_str = "ALTO";
            else if (reservatorio > 33.0f) res_str = "MEDIO";
            else{
                res_str = "BAIXO";
                buzzer_play_tone(BUZZER_ALERT_FREQ, BUZZER_ALERT_ON_MS);
            }
            ssd1306_draw_string(&ssd_global, res_str, col_x + 5, y + 2 + row_h * 4);

            last_oled_update_time = time_us_32();
            //envia os dados para o dispaly
            ssd1306_send_data(&ssd_global); 
        }


        sleep_ms(50);
    }

    cyw43_arch_deinit();
    return 0;
}

// ----- IMPLEMENTAÇÕES DAS FUNÇÕES -----
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK || !newpcb) { printf("Accept Erro %d\n", err); return ERR_VAL; }
    printf("Nova conexao TCP aceita do IP: %s\n", ipaddr_ntoa(&newpcb->remote_ip));
    tcp_arg(newpcb, NULL);
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// user_request atualiza as globais de estado dos relés e a matriz diretamente
void user_request(char **request_str_ptr) {
    if (!request_str_ptr || !(*request_str_ptr)) return;
    char *req = *request_str_ptr;
    printf("USER_REQUEST: Recebido '%s'\n", req);

    if (strstr(req, "GET /irrigacao_on")) {
        led_matrix_draw_water_drop();
        rele_irrigacao = true;
        printf("USER_REQUEST: Irrigacao LIGADA (Matriz: Gota)\n");
    } else if (strstr(req, "GET /irrigacao_off")) {
        rele_irrigacao = false;
        if (!rele_luz) led_matrix_clear(); else led_matrix_draw_light_icon();
        printf("USER_REQUEST: Irrigacao DESLIGADA\n");
    } else if (strstr(req, "GET /luz_on")) {
        led_matrix_draw_light_icon();
        rele_luz = true;
        printf("USER_REQUEST: Luz LIGADA (Matriz: Luz)\n");
    } else if (strstr(req, "GET /luz_off")) {
        rele_luz = false;
        if (!rele_irrigacao) led_matrix_clear(); else led_matrix_draw_water_drop();
        printf("USER_REQUEST: Luz DESLIGADA\n");
    }
}

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

    char requestfer[REQUESTFER_SIZE];
    uint16_t copied_len = pbuf_copy_partial(p, requestfer, sizeof(requestfer) - 1, 0);
    requestfer[copied_len] = '\0';
    pbuf_free(p);

    char *request_ptr_local = requestfer;
    user_request(&request_ptr_local);

    char luminosidade_atual_str[15]; 
    if (luminosidade > 75.0f) { 
        strcpy(luminosidade_atual_str, "ALTA");
    } else if (luminosidade > 25.0f) {
        strcpy(luminosidade_atual_str, "MEDIA");
    } else {
        strcpy(luminosidade_atual_str, "BAIXA");
    }
    char reservatorio_nivel_str[15]; 
    if (reservatorio > 75.0f) {
        strcpy(reservatorio_nivel_str, "ALTO");
    } else if (reservatorio > 25.0f) {
        strcpy(reservatorio_nivel_str, "MEDIO");
    } else {
        strcpy(reservatorio_nivel_str, "BAIXO");
    }

    const char* irrigacao_atual_str = rele_irrigacao ? "LIGADO" : "DESLIGADO";
    const char* luz_atual_str = rele_luz ? "LIGADO" : "DESLIGADO";

    char html[4096];
    snprintf(html, sizeof(html),
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
            "<!DOCTYPE html><html lang=\"pt-BR\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" // META REFRESH REMOVIDO
            "<title>Estação Agro Pico W </title>"
            "<style>body{font-family:Arial,sans-serif;margin:0;padding:20px;background-color:#eef3e8;color:#333}.container{max-width:800px;margin:20px auto;background-color:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}header{text-align:center;margin-bottom:25px;border-bottom:2px solid #4caf50;padding-bottom:15px}header h1{color:#38761d;font-size:2.2em;margin:0}.grid-container{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:15px;margin-bottom:20px}.sensor-card{background-color:#f9f9f9;padding:15px;border-radius:6px;border-left:5px solid #66bb6a; text-align:center;}.sensor-card h2{font-size:1.1em;color:#1b5e20;margin-top:0;margin-bottom:8px}.sensor-card p{margin:5px 0;font-size:0.95em}.value{font-weight:bold;font-size:1.2em;color:#2e7d32}.unit{font-size:0.8em;color:#555}button{font-size:0.8em;padding:6px 10px;margin:5px 2px; border-radius:4px; border:1px solid #ccc; background-color:#f0f0f0; cursor:pointer; min-width:50px;} button:hover{background-color:#e0e0e0;}footer{text-align:center;margin-top:30px;font-size:0.9em;color:#777}</style>"
            "</head><body><div class=\"container\"><header><h1>Monitor Agrícola Inteligente</h1><p style=\"font-size:0.9em; color:#555;\">Fase 2</p></header>"
            "<h2>Condições Atuais</h2><div class=\"grid-container\">"
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
            "<footer><p id=\"footer-info\"><i>PicoW WebServer v1.0 </i></p></footer>"
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
            "// Função para recarregar a página a cada 5 segundos"
            "setInterval(function(){location.href = \"/\"; }, 5000);"
            "window.onload = updateValues;" // Atualiza assim que a página carrega também
            "</script>"
            "</body></html>", // Fechamentos corretos
            temp_ar,
            umid_ar,
            luminosidade_atual_str,
            reservatorio_nivel_str,
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