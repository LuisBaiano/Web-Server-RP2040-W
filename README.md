# Monitor Agrícola Inteligente: Webserver para Visualização e Controle (Fase 1 - simulado)

## Índice

- [Índice](#índice)
- [Objetivos (Fase 1)](#objetivos-fase-1)
- [Descrição do Projeto (Fase 1)](#descrição-do-projeto-fase-1)
- [Funcionalidades Implementadas (Fase 1)](#funcionalidades-implementadas-fase-1)
- [Requisitos Técnicos Atendidos (Fase 1)](#requisitos-técnicos-atendidos-fase-1)
- [Planejamento para Fase 2](#planejamento-para-fase-2)
- [Como Executar](#como-executar)
  - [Requisitos de Hardware](#requisitos-de-hardware)
  - [Requisitos de Software](#requisitos-de-software)
  - [Passos](#passos)
- [Estrutura do Código](#estrutura-do-código)
- [Demonstrativo em Vídeo](#demonstrativo-em-vídeo)

## Objetivos (Fase 1)

* Aplicar os conceitos de desenvolvimento para microcontrolador RP2040 e o módulo Wi-Fi CYW43439 da Raspberry Pi Pico W.
* Desenvolver um servidor web HTTP embarcado capaz de servir uma página HTML dinâmica.
* Implementar a simulação de leitura de sensores ambientais (temperatura do ar, umidade do ar, luminosidade, nível de reservatório) utilizando os periféricos da placa BitDogLab (ADC para Joystick, GPIO para Botões).
* Permitir o controle simulado de atuadores (sistema de irrigação, luz artificial) através de comandos enviados pela interface web.
* Utilizar a Matriz de LEDs WS2812 (via PIO) e o LED RGB integrado (via PWM) para fornecer feedback visual dos estados dos sensores e atuadores simulados, e do status do sistema.
* Utilizar o Display OLED SSD1306 (via I2C) para exibir informações locais sobre o status da conexão, endereço IP e dados simulados.
* Estruturar o código de forma modular e comentada, com configurações centralizadas.
* Servir como base sólida para a Fase 2, que integrará sensores e atuadores reais (DHT22, LDR, Módulo de Relés).

## Descrição do Projeto (Fase 1)

Este projeto, em sua Fase 1, implementa uma "Estação Agrometeorológica" na placa BitDogLab utilizando o RP2040. O sistema simula a coleta de dados ambientais e o controle de atuadores relevantes para agricultura de precisão. Um servidor web HTTP é executado no Pico W, permitindo que um usuário conectado à mesma rede Wi-Fi visualize os dados simulados e acione os atuadores simulados através de uma página web interativa.

**Fluxo de Dados e Controle:**

1. **Simulação de Sensores:**
   * Os eixos X e Y do **Joystick** (ADC) simulam as leituras de **Umidade do Ar** e **Temperatura do Ar**, respectivamente.
   * O **Botão A** (GPIO) simula um sensor de **Luminosidade**, alternando entre "CLARO" e "ESCURO".
   * O **Botão B** (GPIO) simula a variação do **Nível do Reservatório de Água**, ciclando entre "ALTO", "MÉDIO" e "BAIXO".
2. **Interface Web:**
   * Uma página HTML é servida pelo Pico W, exibindo os valores atuais dos sensores simulados e o status dos atuadores simulados (Irrigação, Luz Artificial).
   * A página contém botões "LIGAR"/"DESLIGAR" para cada atuador simulado. Clicar nestes botões envia comandos GET para o servidor.
   * A página utiliza JavaScript para buscar e atualizar os valores exibidos a cada 2 segundos, sem a necessidade de recarregar a página inteira.
3. **Feedback na Placa:**
   * O **Display OLED** mostra informações de status do sistema (Wi-Fi, IP) e uma tabela com os dados simulados atualizados periodicamente.
   * A **Matriz de LEDs WS2812** exibe ícones correspondentes ao estado dos atuadores simulados: um ícone de gota d'água para "Irrigação LIGADA" e um ícone de luz/sol para "Luz Artificial LIGADA".
   * O **LED RGB integrado** indica o estado geral do sistema: Amarelo/Laranja (Conectando Wi-Fi), Vermelho (Erro), Verde (Servidor OK e Rodando), Azul (Comando web recebido). O brilho do LED RGB é controlado via PWM (40% da intensidade máxima).

**Tratamento de Comandos Web:**
A função `user_request` no servidor C interpreta as URLs enviadas pelos botões da página web (ex: `/irrigacao_on`). Com base no comando, ela atualiza o estado do atuador simulado correspondente (alterando uma variável global de estado) e comanda a Matriz de LEDs para exibir o ícone apropriado. O LED RGB também pisca brevemente em azul para confirmar o recebimento do comando.

## Funcionalidades Implementadas (Fase 1)

```
✅ Conexão do Pico W a uma rede Wi-Fi local (Modo Station).
✅ Implementação de um servidor HTTP básico usando LwIP (raw API).
✅ Geração e disponibilização de uma página HTML dinâmica via snprintf.
✅ Atualização automática dos dados na página web a cada 2 segundos utilizando JavaScript (fetch API).
✅ Simulação de leitura de sensores utilizando periféricos da placa:
    ✅ Temperatura do Ar (Joystick Eixo Y).
    ✅ Umidade do Ar (Joystick Eixo X).
    ✅ Luminosidade (Botão A).
    ✅ Nível do Reservatório de Água (Botão B, ciclando entre ALTO/MÉDIO/BAIXO).
✅ Controle simulado de atuadores (Irrigação, Luz Artificial) via botões na página web.
✅ Uso da Matriz de LEDs WS2812 (via PIO) para exibir ícones representando o estado dos atuadores simulados:
    ✅ Ícone de gota d'água para Irrigação LIGADA.
    ✅ Ícone de luz/sol para Luz Artificial LIGADA.
    ✅ Matriz limpa quando os atuadores estão DESLIGADOS.
✅ Uso do LED RGB integrado (via PWM) para feedback de status do sistema:
    ✅ Vermelho Sólido: Erro de Wi-Fi ou sistema.
    ✅ Amarelo/Laranja Sólido (ou Piscando durante a conexão async): Conectando ao Wi-Fi.
    ✅ Verde Sólido: Wi-Fi conectado e servidor rodando.
    ✅ Azul (brevemente): Comando recebido/processado da web.
    ✅ Brilho do LED RGB ajustado para 40%.
✅ Uso do Display OLED SSD1306 (via I2C) para exibir:
    ✅ Tela de inicialização.
    ✅ Status da conexão Wi-Fi e endereço IP.
    ✅ Tabela com os valores dos sensores simulados (atualizada periodicamente).
✅ Implementação de Botão B (GPIO 6, se diferente do botão de simulação) para acionar modo BOOTSEL (opcional, dependendo da configuração de pinos).
✅ Leitura da temperatura interna do microcontrolador RP2040 (exibida apenas no HTML inicial).
✅ Código estruturado em módulos (ex: `main.c`, `display.c`, `buttons.c`, `joystick.c`, `rgb_led.c`, `led_matrix.c`) com headers em `include/`.
✅ Configuração de pinos e constantes centralizada em `include/hardware_config.h`.
✅ Logs de depuração e status enviados via `printf` (stdio/USB).
✅ Tratamento de debounce para os botões A e B.
```

## Requisitos Técnicos Atendidos (Fase 1)

*(Baseado nos critérios de avaliação fornecidos)*

1. **Funcionamento geral do projeto:** O sistema está funcional para os objetivos da Fase 1, permitindo monitoramento e controle simulado via webserver e feedback nos periféricos. (Atendido).
2. **Integração dos periféricos:** Uso coerente e correto do Joystick (ADC), Botões (GPIO com debounce), Display OLED (I2C), Matriz de LEDs (PIO), LED RGB (PWM), e principalmente do módulo Wi-Fi CYW43439. (Atendido).
3. **Organização e clareza do código:** O código está estruturado em múltiplos arquivos `.c` e `.h`, com configurações em `hardware_config.h`. Comentários serão mantidos/adicionados para fácil entendimento. (Em processo, visa atender).
4. **Implementação técnica:** Uso adequado de Wi-Fi, ADC, UART (para logs), e tratamento de debounce. (Atendido).
5. **Criatividade e originalidade:** A aplicação de um webserver para monitoramento agrícola, mesmo que simulado, com feedback visual na matriz de LEDs e controle via web apresenta uma proposta interessante e funcional para o escopo do Pico W. (Atendido).

## Planejamento para Fase 2

* Integrar sensores reais: DHT22 (temperatura e umidade do ar), LDR (luminosidade).
* Integrar o módulo de relés de 4 canais para controlar atuadores reais (bomba de irrigação, luz de crescimento).
* Implementar lógica de automação básica (ex: ligar irrigação se umidade do solo baixa).
* Utilizar o Buzzer para alertas sonoros de condições críticas.
* (Opcional) Refinar a interface web.

## Como Executar

### Requisitos de Hardware

* Placa de desenvolvimento **BitDogLab** (com RP2040 e chip Wi-Fi CYW43439).
* Cabo Micro-USB para conexão e alimentação/programação.
* Computador ou dispositivo móvel na mesma rede Wi-Fi para acessar a página web.

### Requisitos de Software

* **VS Code** com a extensão Pico-W-Go ou configuração manual do toolchain ARM e Pico SDK.
* **Pico SDK** (ex: v1.5.1 ou compatível) instalado e configurado.
* **LwIP (Lightweight IP stack):** Vem com o Pico SDK.
* **Git** (opcional, para clonar).
* Um **Terminal Serial** (ex: Monitor Serial do VS Code, Putty, Minicom) configurado para a porta serial da Pico (baudrate 115200).

### Passos

1. **Configurar Credenciais Wi-Fi:** Edite o arquivo `main.c` e substitua os placeholders em `#define WIFI_SSID "SEU_SSID"` e `#define WIFI_PASSWORD "SUA_SENHA"` pelas credenciais da sua rede.
2. **Configurar Pinos (se necessário):** Verifique e ajuste os pinos definidos em `include/hardware_config.h` para corresponderem à sua placa BitDogLab.
3. **Compilar (Build):**
   * Certifique-se de que o `CMakeLists.txt` inclui todos os arquivos `.c` necessários (main.c, display.c, buttons.c, joystick.c, rgb_led.c, led_matrix.c, ssd1306.c, font.c) e as bibliotecas (`pico_stdlib`, `hardware_i2c`, `hardware_adc`, `hardware_pio`, `pico_cyw43_arch_lwip_threadsafe_background`, etc.).
   * Use a função de build do VS Code ou compile via linha de comando:
     ```bash
     mkdir build
     cd build
     cmake ..
     make
     ```
4. **Carregar o Firmware:**
   * Coloque a BitDogLab em modo BOOTSEL (pressione e segure o botão BOOTSEL enquanto conecta o USB, ou use o botão de reset para BOOTSEL.
   * Copie o arquivo `.uf2` gerado (ex: `pico_webserver.uf2`) da pasta `build` para o drive `RPI-RP2`.
   * A placa reiniciará automaticamente.
5. **Visualizar Logs:** Abra o terminal serial na porta COM correta com 115200 baud. Observe as mensagens de inicialização, conexão Wi-Fi e o endereço IP.
6. **Testar:**
   * Após a mensagem "Servidor OK, Ouvindo..." e o IP ser exibido, acesse o endereço IP do Pico W em um navegador na mesma rede Wi-Fi.
   * Interaja com o Joystick e os Botões A/B na placa para ver os dados simulados mudarem no Display OLED e na página web (que deve atualizar via JavaScript a cada 2 segundos).
   * Clique nos botões "LIGAR"/"DESLIGAR" na página web para "Irrigação" e "Luz Artificial" e observe a Matriz de LEDs e o LED RGB na placa mudarem.

## Estrutura do Código

```
.
├── include/
│   ├── config.h            # Definições de pinos e constantes
│   ├── display.h           # Headers para display OLED
│   ├── buttons.h
│   ├── joystick.h
│   ├── rgb_led.h
│   ├── led_matrix.h
│   └── lib/
│       └── ssd1306/        # Biblioteca do driver SSD1306
│           ├── ssd1306.c
│           ├── ssd1306.h
│           └── font.h  
│   ├── display.c
│   ├── buttons.c
│   ├── joystick.c
│   ├── rgb_led.c
│   ├── led_matrix.c
│   └── (outros módulos como buzzer.c se implementado)
├── /pio/
│   └── led_matrix.pio      # Programa PIO para a matriz de LEDs
├── main.c                  # Lógica principal, servidor web, loop
├── lwipopts.h              # Configurações da LwIP (importante!)
├── CMakeLists.txt          # Script de build do CMake
└── pico_sdk_import.cmake   # Padrão do SDK
```

*(A estrutura exata dos diretórios `src/` vs. arquivos `.c` na raiz depende da sua preferência de organização).*

## Demonstrativo em Vídeo

[[Link para o seu Vídeo de Demonstração da Atividade 1]](https://drive.google.com/drive/folders/1CdSdQ_Fl-S2tXVT6iayEbYfwUgxTDNqq?usp=sharing)
