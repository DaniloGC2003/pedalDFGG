# Formato dos pacotes na comunicação serial Arduino-ESP32

Os pacotes seriais são compostos de um header seguido de 8 bytes: **"ESP" + message_id bytes**
Os códigos de mensagem atualmente implementados são:
- UPDATE_ENCODER_MESSAGE_ID 11. Mensagem para atualização de um dos parâmetros dos encoders/controles deslizantes. A mensagem consiste em 2 bytes: o primeiro representa o parâmetro a ser atualizado; o segundo representa o valor a ser configurado no parâmetro.
- UPDATE_ALL_ENCODERS_MESSAGE_ID 12. Mensagem para atualização de todos os parâmetros dos encoders/controles deslizantes. A mensagem consiste em 4 bytes. Cada um deles representa o valor de um parâmetro diferente.

## ESP32 -> Arduino
- UPDATE_ENCODER_MESSAGE_ID: Uma mensagem da ESP32 para o Arduino é sempre enviada quando a ESP32 recebe uma mensagem de atualização de parâmetros dos encoders (UPDATE_ENCODER_MESSAGE_ID).
    - Formato: "ESP" + ID + ID_encoder + valor_encoder

- UPDATE_ALL_ENCODERS_MESSAGE_ID: A ESP32 também pode enviar ao Arduino mensagens com o código UPDATE_ALL_ENCODERS_MESSAGE_ID. Nesse caso, o Arduino é encarregado de enviar à ESP32 todos os atuais valores referentes aos parâmetros de cada encoder.
    - Formato: "ESP" + ID

## Arduino -> ESP32
- UPDATE_ENCODER_MESSAGE_ID: Uma mensagem de ID UPDATE_ENCODER_MESSAGE_ID conterá o ID e o valor atual referente a um dos parâmetros dos encoders do Arduino. A ESP32, por sua vez, deverá repassar essa informação à página web, a fim de que os valores sejam atualizados ao usuário.
    - Formato: "ESP" + ID + ID_encoder + valor_encoder

- UPDATE_ALL_ENCODERS_MESSAGE_ID: Uma mensagem de ID UPDATE_ALL_ENCODERS_MESSAGE_ID do Arduino para a ESP32 é uma resposta do Arduino a uma mensagem anteriormente enviada com o mesmo ID. Essa mensagem recebida pela ESP32 contém todos os valores atuais dos encoders. Esses valores serão repassados via BLE para que a página seja atualizada.
    - Formato: "ESP" + ID + valor0 + valor1 + valor2 + valor3

# Formato dos pacotes na comunicação BLE ESP32-página web

A comunicação BLE utiliza o serviço de UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b" e possui uma única característica de UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8". A troca de mensagens entre ESP32 e página web ocorre através de pacotes de 8 bytes. Obrigatoriamente, o primeiro byte deve ser reservado para o ID da mensagem.

## ESP32 -> página web
- UPDATE_ENCODER_MESSAGE_ID: Uma mensagem BLE para a página web com esse ID contém dados atualizados que serão inseridos em um dos quatro controles deslizantes da página. A mensagem contém 2 bytes: o ID do slider e o valor a ser escrito nele.
    - Formato: ID + ID_encoder + valor_encoder

- UPDATE_ALL_ENCODERS_MESSAGE_ID: Uma mensagem BLE com esse ID contém dados atualizados que serão inseridos em todos os sliders da página. A mensagem contém 4 bytes, cada um referente a um slider.
    - Formato: ID + valor0 + valor1 + valor2 + valor3

## Página web -> ESP32
- UPDATE_ENCODER_MESSAGE_ID: Essa mensagem contém o identificador e o valor de um dos 4 sliders da página. Esses valores devem ser repassados para a ESP32 e, finalmente, para o Arduino, para que ele possa atualizar tal informação.
    - Formato: ID + ID_encoder + valor_encoder

- UPDATE_ALL_ENCODERS_MESSAGE_ID: Essa mensagem é uma requisição de todos os valores dos encoders do Arduino. Mensagens com esse ID somente são enviadas após sucesso na conexão entre ESP32 e página web. A ESP32 deve repassar essa mensagem ao Arduino, que deverá informar os valores em seus encoders. 
    - Formato: ID + valor0 + valor1 + valor2 + valor3