#include <Wire.h>
#include <Mouse.h>

/**
 * @brief Variável para armazenar o valor do eixo X do joystick do Nunchuck (0-255).
 */
uint8_t joyX;

/**
 * @brief Variável para armazenar o valor do eixo Y do joystick do Nunchuck (0-255).
 */
uint8_t joyY;

/**
 * @brief Variável para armazenar o estado do botão Z do Nunchuck (0 para pressionado, 1 para não pressionado).
 */
uint8_t zButton;

/**
 * @brief Variável para armazenar o estado do botão C do Nunchuck (0 para pressionado, 1 para não pressionado).
 */
uint8_t cButton;

/**
 * @brief Função de configuração que é executada uma vez no início do programa.
 *
 * Inicializa a comunicação serial, a interface I2C (Wire) e a funcionalidade do mouse.
 * Também inicializa o Nunchuck para começar a receber dados corretamente.
 */
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Mouse.begin();

  // Inicialização do Nunchuck
  Wire.beginTransmission(0x52); // Inicia a comunicação com o endereço I2C do Nunchuck
  Wire.write(0xF0);             // Envia o comando para desativar a criptografia (necessário para alguns Nunchucks)
  Wire.write(0x55);
  Wire.endTransmission();       // Finaliza a transmissão
  delay(1);

  Wire.beginTransmission(0x52); // Inicia novamente a comunicação
  Wire.write(0xFB);             // Envia o comando para ler os dados como dados brutos
  Wire.write(0x00);
  Wire.endTransmission();       // Finaliza a transmissão
  delay(1);
}

/**
 * @brief Função para ler os dados do Nunchuck.
 *
 * Envia um pedido de leitura de dados para o Nunchuck através da interface I2C.
 * Os dados recebidos incluem as posições dos eixos X e Y do joystick e o estado dos botões Z e C.
 * Os valores dos acelerômetros são lidos, mas ignorados neste código.
 */
void readNunchuck() {
  Wire.beginTransmission(0x52); // Inicia a comunicação com o Nunchuck
  Wire.write(0x00);             // Solicita o envio dos dados
  Wire.endTransmission();       // Finaliza a transmissão
  delay(1);

  Wire.requestFrom(0x52, 6);    // Solicita 6 bytes de dados do Nunchuck
  if (Wire.available() >= 6) {  // Verifica se 6 bytes foram recebidos
    joyX = Wire.read();        // Lê o valor do eixo X do joystick
    joyY = Wire.read();        // Lê o valor do eixo Y do joystick
    Wire.read();              // Lê o valor do acelerômetro X (ignorado)
    Wire.read();              // Lê o valor do acelerômetro Y (ignorado)
    Wire.read();              // Lê o valor do acelerômetro Z (ignorado)
    uint8_t buttons = Wire.read(); // Lê o byte contendo o estado dos botões

    zButton = !(buttons & 0x01);      // Extrai o estado do botão Z (bit 0). 0 se pressionado.
    cButton = !((buttons >> 1) & 0x01); // Extrai o estado do botão C (bit 1). 0 se pressionado.
  }
}

/**
 * @brief Função principal de loop que é executada continuamente após a função de configuração.
 *
 * Lê os dados do Nunchuck, calcula o movimento do mouse com base na posição do joystick,
 * aplica uma zona morta para evitar movimentos involuntários, move o cursor do mouse e
 * simula cliques de botão com os botões Z e C do Nunchuck.
 */
void loop() {
  readNunchuck();

  // Calcular movimento do mouse (ajusta a sensibilidade aqui)
  int xMove = (int)joyX - 128; // Calcula o movimento relativo no eixo X (-128 a 127)
  int yMove = (int)joyY - 128; // Calcula o movimento relativo no eixo Y (-128 a 127)

  // Inverter eixo Y se necessário (testa para ver qual está melhor)
  yMove = -yMove;

  // Aplica um deadzone para evitar tremores
  if (abs(xMove) < 10) xMove = 0;
  if (abs(yMove) < 10) yMove = 0;

  // Mover o mouse baseado no joystick
  if (xMove != 0 || yMove != 0) {
    Mouse.move(xMove / 7, yMove / 7, 0); // Divide para reduzir a velocidade do movimento do mouse
  }

  // Clique com os botões
  if (zButton) {
    if (!Mouse.isPressed(MOUSE_LEFT)) Mouse.press(MOUSE_LEFT); // Pressiona o botão esquerdo do mouse se Z estiver pressionado e não estiver pressionado ainda
  } else {
    if (Mouse.isPressed(MOUSE_LEFT)) Mouse.release(MOUSE_LEFT); // Libera o botão esquerdo do mouse se Z não estiver pressionado e estiver pressionado
  }

  if (cButton) {
    if (!Mouse.isPressed(MOUSE_RIGHT)) Mouse.press(MOUSE_RIGHT); // Pressiona o botão direito do mouse se C estiver pressionado e não estiver pressionado ainda
  } else {
    if (Mouse.isPressed(MOUSE_RIGHT)) Mouse.release(MOUSE_RIGHT); // Libera o botão direito do mouse se C não estiver pressionado e estiver pressionado
  }

  delay(20); // Pequeno atraso para evitar leitura excessiva e estabilizar o sistema
}