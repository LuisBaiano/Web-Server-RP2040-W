
# Monitoramento Agrícola Inteligente: Webserver com Sensores Reais (Fase Final)

## Índice

- [Monitoramento Agrícola Inteligente: Webserver com Sensores Reais (Fase Final)](#monitoramento-agrícola-inteligente-webserver-com-sensores-reais-fase-final)
  - [Índice](#índice)
  - [Objetivos (Fase Final)](#objetivos-fase-final)
  - [Descrição do Projeto (Fase Final)](#descrição-do-projeto-fase-final)
  - [Funcionalidades Implementadas (Fase Final)](#funcionalidades-implementadas-fase-final)
  - [Requisitos Técnicos Atendidos (Fase Final)](#requisitos-técnicos-atendidos-fase-final)
  - [Como Executar](#como-executar)
    - [Requisitos de Hardware](#requisitos-de-hardware)
    - [Requisitos de Software](#requisitos-de-software)
    - [Passos](#passos)
  - [Estrutura do Código](#estrutura-do-código)
  - [Demonstrativo em Vídeo](#demonstrativo-em-vídeo)

## Objetivos (Fase Final)

* Desenvolver um servidor web embarcado robusto na Raspberry Pi Pico W, capaz de coletar e apresentar dados de sensores ambientais reais (temperatura do ar, umidade do ar com DHT22; luminosidade com LDR).
* Implementar a simulação interativa de parâmetros adicionais (nível de reservatório de água via Joystick) para enriquecer o modelo agrícola.
* Disponibilizar os dados de monitoramento e permitir o controle de atuadores simulados (irrigação, luz artificial) através de uma interface web dinâmica (HTML com JavaScript), acessível por celular ou computador na rede local.
* Fornecer feedback visual em tempo real do estado dos sensores e atuadores nos periféricos da placa BitDogLab (Display OLED, Matriz de LEDs WS2812, LED RGB).
* Adicionar alertas sonoros para condições críticas (buzzer para nível de reservatório baixo).
* Consolidar conhecimentos em programação C para sistemas embarcados, comunicação Wi-Fi (módulo CYW43439), interação com diversos periféricos (ADC, GPIO, I2C, PIO).

## Descrição do Projeto (Fase Final)

Este projeto, na sua versão final, implementa uma "Estação Agrometeorológica Inteligente" na placa BitDogLab, utilizando o microcontrolador RP2040 e a conectividade Wi-Fi. O sistema realiza o monitoramento ambiental através de sensores reais e disponibiliza esses dados via um webserver HTTP embarcado no Pico W. O usuário pode acessar uma página web interativa para visualizar as condições ambientais atuais e controlar (simuladamente) sistemas de irrigação e iluminação artificial, com o feedback visual dessas ações diretamente na Matriz de LEDs da placa. O projeto demonstra a aplicação de sistemas embarcados conectados para soluções de agricultura de precisão.

**Fluxo de Dados e Controle:**

1. **Coleta de Dados de Sensores (Reais e Simulados):**
   * **DHT22 (Sensor Externo):** Coleta as leituras reais de **Temperatura do Ar** e **Umidade do Ar**.
   * **LDR (Sensor Externo):** Coleta a leitura real de **Luminosidade ambiente**.
   * **Joystick Eixo X (ADC):** Simula o **Nível do Reservatório de Água**, permitindo ao usuário alterar o nível (Baixo, Médio, Alto).
   * *(Joystick Eixo Y é utilizado para calibrar a temperatura e umidade ou pode ser desconsiderado na coleta de dados principais).*
2. **Interface Web:**
   * Uma página HTML (`index.html`) é servida pelo Pico W, exibindo os dados atualizados dos sensores e o status dos atuadores.
   * A página possui botões "LIGAR"/"DESLIGAR" para os atuadores simulados ("Irrigação", "Luz Artificial"). Estes botões enviam comandos GET para o servidor.
   * O JavaScript na página web realiza buscas periódicas (a cada 2 segundos) pelo conteúdo da página e atualiza os valores sem recarregar a tela inteira (AJAX).
3. **Feedback na Placa:**
   * **Display OLED (I2C):** Exibe o status da conexão Wi-Fi (IP, OK/Erro) e uma tabela com os dados de Temperatura, Umidade, Luminosidade e Nível do Reservatório.
   * **Matriz de LEDs WS2812 (PIO):** Visualiza o estado dos atuadores simulados:
     * Ícone de **gota d'água** quando "Irrigação" LIGADA.
     * Ícone de **luz/sol** quando "Luz Artificial" LIGADA.
     * É limpa quando os atuadores estão DESLIGADOS.
   * **LED RGB integrado (PWM):** Indica o status geral do sistema:
     * Roxo: Inicializando periféricos.
     * Amarelo/Laranja: Conectando Wi-Fi.
     * Verde: Wi-Fi conectado e servidor operacional.
     * Vermelho: Erro fatal (Wi-Fi ou servidor).
     * Azul (breve): Comando web de atuador recebido.
   * **Buzzer:** Emite um alerta sonoro quando o nível simulado do reservatório atinge o estado "BAIXO".

**Tratamento de Comandos Web:**
A função `user_request` (no servidor C) processa os comandos recebidos da página web (ex: `/irrigacao_on`), atualiza as variáveis de estado dos atuadores (`rele_irrigacao`, `rele_luz`), e comanda diretamente a Matriz de LEDs para exibir o ícone correspondente.

## Funcionalidades Implementadas (Fase Final)

```
✅ Conexão do Pico W a uma rede Wi-Fi local (Modo Station) com tratamento de erro.
✅ Implementação de um servidor HTTP básico usando LwIP (raw API).
✅ Geração e disponibilização de uma página HTML dinâmica (`snprintf`) com controle de atuadores.
✅ **Atualização automática e dinâmica dos dados na página web** a cada 2 segundos utilizando JavaScript (Fetch API, DOMParser).
✅ **Leitura e processamento de sensores REAIS:**
    ✅ **Temperatura do Ar (DHT22):** Obtenção de valor em °C.
    ✅ **Umidade do Ar (DHT22):** Obtenção de valor em %.
    ✅ **Luminosidade (LDR):** Obtenção de valor em porcentagem ou categorias (ALTA/MÉDIA/BAIXA).
✅ Implementação de **simulação interativa de Nível do Reservatório de Água** utilizando o Joystick Eixo X (mapeamento para ALTO/MÉDIO/BAIXO).
✅ Controle simulado de atuadores (Irrigação, Luz Artificial) via botões na página web.
✅ Uso da Matriz de LEDs WS2812 (via PIO) para exibir ícones correspondentes ao estado dos atuadores simulados (Gota d'água para irrigação, Luz/Sol para luz artificial).
✅ Uso do LED RGB integrado (via PWM) para feedback visual de status do sistema (Roxo/Amarelo/Verde/Vermelho) e comando web (Azul breve).
✅ Uso do Display OLED SSD1306 (via I2C) para exibir: Tela de inicialização, status Wi-Fi/IP, e uma tabela periódica de dados (reais e simulados).
✅ Alerta Sonoro no Buzzer: Emite um som quando o nível do reservatório atinge o estado "BAIXO".
✅ Implementação do Botão B para acionar modo BOOTSEL para fácil reprogramação.
✅ Implementação de leitura de botões (A e B) com tratamento de debounce (IRQs para Botão A e B, dependendo da configuração).
✅ Código estruturado em módulos (`main.c`, `display.c`, `buttons.c`, `joystick.c`, `rgb_led.c`, `led_matrix.c`, `dht22.c`, `ldr.c`, `buzzer.c`) com headers em `include/`.
✅ Configuração de pinos e constantes centralizada em `include/hardware_config.h`.
✅ Logs de depuração e status detalhados via `printf` (stdio/USB).
```

## Requisitos Técnicos Atendidos (Fase Final)

*(Conforme Tabela 1 de Critérios de Avaliação)*

1. **Funcionamento geral do projeto:** O sistema é totalmente funcional, cumpre os objetivos de monitoramento com sensores reais e controle simulado, e utiliza todos os periféricos planejados. (Atendido: 20%).
2. **Integração dos periféricos:** Realiza o uso coerente e correto de múltiplos periféricos (Joystick, Botões, Display OLED, Matriz de LEDs, LED RGB, Buzzer) e integra o módulo Wi-Fi CYW43439 para a funcionalidade de servidor web. (Atendido: 15%).
3. **Organização e clareza do código:** O código está bem estruturado em módulos, com indentação consistente, arquivos organizados em `include/` e `src/` (ou similar), e comentários úteis, facilitando o entendimento. (Atendido: 15%).
4. **Implementação técnica:** Utiliza adequadamente Wi-Fi (servidor HTTP), ADC (Joystick, LDR), GPIOs, I2C, PIO, timers (para DHT22), e implementa tratamento de debounce para os botões. (Atendido: 15%).
5. **Criatividade e originalidade:** O projeto apresenta uma proposta criativa e funcional para monitoramento agrícola utilizando um webserver embarcado no Pico W, com a adição de sensores reais e feedback visual/sonoro interativo. (Atendido: 10%).

## Como Executar

### Requisitos de Hardware

* Placa de desenvolvimento **BitDogLab** (com RP2040 e chip Wi-Fi CYW43439).
* Cabo Micro-USB para conexão e alimentação/programação.
* Sensores externos: **DHT22**, **LDR** (com divisor de tensão).
* Dispositivo de rede: Computador ou dispositivo móvel na mesma rede Wi-Fi que o Pico W.

### Requisitos de Software

* **VS Code** com a extensão Pico-W-Go ou configuração manual do toolchain ARM e Pico SDK.
* **Pico SDK** (ex: v1.5.1 ou compatível) instalado e configurado.
* **LwIP (Lightweight IP stack):** Inclusa no Pico SDK.
* **Git** (opcional, para clonar).
* Um **Terminal Serial** (ex: Monitor Serial do VS Code, Putty, Minicom) configurado para a porta serial da Pico (baudrate 115200).

### Passos

1. **Configurar Credenciais Wi-Fi:** Edite o arquivo `main.c` (ou `hardware_config.h`, se centralizado) e substitua os placeholders em `#define WIFI_SSID "SEU_SSID"` e `#define WIFI_PASSWORD "SUA_SENHA"` pelas credenciais da sua rede Wi-Fi.
2. **Configurar Pinos de Hardware:** Verifique e ajuste todos os pinos GPIO (`DHT_PIN`, `LDR_ADC_PIN`, `BUTTON_A_PIN`, `BUTTON_B_PIN`/`BOOTSEL_BUTTON_PIN`, `JOYSTICK_X_ADC_CHANNEL`, `JOYSTICK_Y_ADC_CHANNEL`, `LED_RED_PIN`/`GREEN`/`BLUE`, `MATRIX_WS2812_PIN`, `BUZZER_PIN_1`, `I2C_SDA_PIN`/`SCL`) definidos em `include/hardware_config.h` para corresponderem às conexões físicas da sua placa BitDogLab e dos sensores externos.
3. **Compilar (Build):**
   * Certifique-se de que o `CMakeLists.txt` inclui todos os arquivos `.c` necessários para a compilação do projeto final: `main.c`, `display.c`, `buttons.c`, `joystick.c`, `rgb_led.c`, `led_matrix.c`, `dht22.c`, `ldr.c`, `buzzer.c`, `ssd1306.c`, `font.c` e quaisquer outros que você tenha criado.
   * Use a função de build do VS Code ou compile via linha de comando:
     ```bash
     mkdir build
     cd build
     cmake ..
     make -j$(nproc)
     ```
4. **Carregar o Firmware:**
   * Conecte a BitDogLab ao computador. Para entrar no modo BOOTSEL, pressione e segure o botão físico correspondente ao `BOOTSEL_BUTTON_PIN` (se configurado) ou o botão BOOTSEL da placa (geralmente próximo ao micro-USB) enquanto conecta o cabo USB (ou reinicie).
   * Copie o arquivo `.uf2` gerado (ex: `pico_webserver.uf2`) da pasta `build` para o drive `RPI-RP2` que aparece.
   * A placa reiniciará automaticamente com o novo firmware.
5. **Visualizar Logs:** Abra um terminal serial (ex: Monitor Serial do VS Code) configurado para a porta COM correta do Pico e **115200 baud**. Observe as mensagens de inicialização, conexão Wi-Fi e o endereço IP obtido.
6. **Testar a Aplicação:**
   * Após a mensagem "Servidor OK, Aguardando..." e o IP ser exibido, abra um navegador (em computador ou celular) conectado à mesma rede Wi-Fi e digite o endereço IP do Pico W (ex: `http://192.168.1.XX`).
   * Observe a página web se atualizar automaticamente a cada 2 segundos com os dados reais dos sensores (DHT22, LDR) e o nível do reservatório (Joystick).
   * Interaja com o Joystick para alterar o nível do reservatório e veja a atualização na página e o alerta do buzzer.
   * Clique nos botões "LIGAR"/"DESLIGAR" na página web para "Irrigação" e "Luz Artificial". Observe a Matriz de LEDs e o LED RGB integrarem o feedback.
   * Pressione o Botão A para alternar o estado de luminosidade (se a lógica ainda usa o botão A para simulação).

## Estrutura do Código

```
.
├── include/
│   ├── hardware_config.h   # Definições de pinos, constantes e buffers
│   ├── display.h           # Headers para display OLED
│   ├── buttons.h
│   ├── joystick.h
│   ├── rgb_led.h
│   ├── led_matrix.h
│   ├── dht22.h             # Header para o sensor DHT22
│   ├── ldr.h               # Header para o sensor LDR
│   ├── buzzer.h            # Header para o Buzzer
│   └── lib/                # Bibliotecas externas (SSD1306)
│       └── ssd1306/
│           ├── ssd1306.c
│           ├── ssd1306.h
│           └── font.h
├── src/                    # Implementações dos módulos
│   ├── display.c
│   ├── buttons.c
│   ├── joystick.c
│   ├── rgb_led.c
│   ├── led_matrix.c
│   ├── dht22.c             # Código do sensor DHT22
│   ├── ldr.c               # Código do sensor LDR
│   └── buzzer.c            # Código do Buzzer
├── pio_programs/           # Programas PIO Assembly (se em pasta separada)
│   └── led_matrix.pio
├── main.c                  # Lógica principal, servidor web, loop de execução
├── lwipopts.h              # Configurações da LwIP (TCP/IP)
├── CMakeLists.txt          # Script de build do CMake
└── pico_sdk_import.cmake   # Padrão do Pico SDK
```

## Demonstrativo em Vídeo

[[Link para o seu Vídeo de Demonstração (Atividade 2 / Final)]](https://drive.google.com/drive/folders/1CdSdQ_Fl-S2tXVT6iayEbYfwUgxTDNqq?usp=sharing)
