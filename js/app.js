const connectBTN = document.querySelector(".connect");
const disconnectBTN = document.querySelector(".disconnect");
const sendBTN = document.querySelector(".send"); // Botao usado para fins de debugging. Nao deve aparecer na versao oficial.
const errorTxt = document.querySelector(".error");
const bt_connection_icon = document.querySelector(".icon-connection");
const text_connection = document.querySelector(".text-connection");
const slider_containers = document.querySelectorAll(".slider-container");
const encoder0 = document.querySelector("#encoder0");
const encoder1 = document.querySelector("#encoder1");
const encoder2 = document.querySelector("#encoder2");
const clk = document.querySelector("#clk");

const UPDATE_ENCODER_MESSAGE_ID = 11;
const UPDATE_ALL_ENCODERS_MESSAGE_ID = 12;

let device;
let messageCharacteristic;

const ble_payload = new Uint8Array(8);
let busy_ble = false;

// Register the service worker
/*if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('/sw.js')
    .then((reg) => console.log('SW registered', reg))
    .catch((error) => console.error('SW registration failed', error));
}*/

slider_containers.forEach(container => {
    const slider = container.querySelector('input[type="range"]');
    const value = container.querySelector('p');

    const updateValue = () => {
        value.textContent = slider.value;
        /*if (messageCharacteristic){
            if (container.id == "encoder0") {
                ble_payload[0] = 0x00;
            }
            else if (container.id == "encoder1") {
                ble_payload[0] = 0x01;
            }
            else if (container.id == "encoder2") {
                ble_payload[0] = 0x02;
            }
            else if (container.id == "clk") {
                ble_payload[0] = 0x03;
            }
            ble_payload[1] = slider.value & 0xFF;
            console.log("to be sent: " + ble_payload);
            sendData(ble_payload);
        }*/
    }
    const updateValueOnRelease = () => {
        if (messageCharacteristic){
            ble_payload.fill(0);
            ble_payload[0] = UPDATE_ENCODER_MESSAGE_ID;
            if (container.id == "encoder0") {
                ble_payload[1] = 0x00;
            }
            else if (container.id == "encoder1") {
                ble_payload[1] = 0x01;
            }
            else if (container.id == "encoder2") {
                ble_payload[1] = 0x02;
            }
            else if (container.id == "clk") {
                ble_payload[1] = 0x03;
            }
            ble_payload[2] = slider.value & 0xFF;
            console.log("to be sent: " + ble_payload);
            sendData(ble_payload);
        }
    }
    updateValue();
    slider.addEventListener('input', updateValue);
    slider.addEventListener('change', updateValueOnRelease);
})

function update_all_sliders_ble(bytes) {
    console.log("changing ble data");
    encoder0.querySelector('input[type="range"]').value = bytes[0];
    encoder1.querySelector('input[type="range"]').value = bytes[1];
    encoder2.querySelector('input[type="range"]').value = bytes[2];
    clk.querySelector('input[type="range"]').value = bytes[3];

    encoder0.querySelector('p').textContent = bytes[0];
    encoder1.querySelector('p').textContent = bytes[1];
    encoder2.querySelector('p').textContent = bytes[2];
    clk.querySelector('p').textContent = bytes[3];
}

function update_single_slider_ble(bytes) {
    if (bytes[0] == 0x00) {
        encoder0.querySelector('input[type="range"]').value = bytes[1];
        encoder0.querySelector('p').textContent = bytes[1];
    }
    else if (bytes[0] == 0x01) {
        encoder1.querySelector('input[type="range"]').value = bytes[1];
        encoder1.querySelector('p').textContent = bytes[1];
    }
    else if (bytes[0] == 0x02) {
        encoder2.querySelector('input[type="range"]').value = bytes[1];
        encoder2.querySelector('p').textContent = bytes[1];
    }
    else if (bytes[0] == 0x03) {
        clk.querySelector('input[type="range"]').value = bytes[1];
        clk.querySelector('p').textContent = bytes[1];
    }
}


function handleReceivedData(event) {
    console.log("mensagem recebida\n");
    const value = event.target.value;   
    for (let i = 0; i < value.byteLength; i++) {
        console.log(value.getUint8(i));
    }

    let bytes = [];
    if (value.getUint8(0) == UPDATE_ALL_ENCODERS_MESSAGE_ID) {
        for (let i = 1; i < value.byteLength; i++)
            bytes.push(value.getUint8(i));
        update_all_sliders_ble(bytes);
    }
    else if (value.getUint8(0) == UPDATE_ENCODER_MESSAGE_ID) {
        bytes.push(value.getUint8(1));
        bytes.push(value.getUint8(2));
        update_single_slider_ble(bytes);
    }
    
    


}


async function connectDevice() {
    console.log("connecting..."); 

    if (device.gatt.connected) return; 

    const server = await device.gatt.connect(); 

    await new Promise(resolve => setTimeout(resolve, 500)); // Delay to ensure stable connection.   

    // Recupera o serviço primário usando o UUID específico.
    const service = await server.getPrimaryService("4fafc201-1fb5-459e-8fcc-c5c9c331914b"); 

    // Adquire a característica de mensagem.
    messageCharacteristic = await service.getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8"); 

    // Adiciona o listener para lidar com dados recebidos.
    messageCharacteristic.addEventListener("characteristicvaluechanged", handleReceivedData); 

    // Atualiza UI
    disconnectBTN.classList.remove("hide"); 
    connectBTN.classList.add("hide"); 
    bt_connection_icon.textContent = "bluetooth_connected"; 
    text_connection.textContent = "Device connected"; 
    console.log("connected"); 

    console.log("requesting latest values for sliders"); 
    ble_payload.fill(0); 
    ble_payload[0] = UPDATE_ALL_ENCODERS_MESSAGE_ID; 
    sendData(ble_payload); 
}

async function disconnectDevice() {
    // Verificar se o dispositivo foi definido e se ele está conectado [4]
    if (device && device.gatt.connected) { 
        console.log("Disconnecting..."); 
        device.gatt.disconnect(); 
        console.log("BLE device disconnected"); 

        messageCharacteristic = null;
        device = null; 

        if (connectBTN) { 
            connectBTN.textContent = "connect"; 
        }
    }

    // Atualiza UI
    disconnectBTN.classList.add("hide");
    connectBTN.classList.remove("hide"); 
    bt_connection_icon.textContent = "bluetooth_disabled"; 
    text_connection.textContent = "No device connected"; 
}

async function requestDevice() {
    const options = {
        acceptAllDevices: true,
        optionalServices: ["4fafc201-1fb5-459e-8fcc-c5c9c331914b"]
    };

    device = await navigator.bluetooth.requestDevice(options); 

    device.addEventListener("gattserverdisconnected", init);
}

async function startMonitoring() {
    // Se a conexão for estável até este ponto, a característica é válida.
    await messageCharacteristic.startNotifications(); 
}


async function sendData(data) {
    if (!messageCharacteristic) {
        console.error("Characteristic not intialized.");
        return;
    }

    // ignore message if data is currently being sent
    if (busy_ble) {
        console.log("BLE busy, message ignored");
        return;
    }

    busy_ble = true;
    await messageCharacteristic.writeValue(data);
    busy_ble = false;

    console.log("Data " + data + " sent successfully.");
}


async function init() {
    console.log("Init function called");
    if (!navigator.bluetooth) return errorTxt.classList.remove("hide"); 

    // Tenta obter o dispositivo se não estiver definido (primeira conexão ou após desconexão manual).
    if (!device) await requestDevice();

    try {
        // Atualiza a UI para o estado de "conectando" 
        connectBTN.textContent = "connecting..."; 
        bt_connection_icon.textContent = "bluetooth_searching"; 
        text_connection.textContent = "Connecting" 

        await connectDevice(); // 1. Conecta e adquire a characteristic
        await startMonitoring(); // 2. Inicia as notificações

    } catch (error) {
        console.error("Connection or initialization failed:", error);
        
        // Em caso de qualquer falha durante a conexão/monitoramento, 
        // reseta o estado da aplicação e a UI, chamando a rotina de desconexão.
        disconnectDevice(); 
    }
}

connectBTN.addEventListener("click", init);
disconnectBTN.addEventListener("click", disconnectDevice);
sendBTN.addEventListener("click", sendData.bind(null, 5));