const connectBTN = document.querySelector(".connect");
const disconnectBTN = document.querySelector(".disconnect");
const sendBTN = document.querySelector(".send");
const errorTxt = document.querySelector(".error");
const bt_connection_icon = document.querySelector(".icon-connection");
const text_connection = document.querySelector(".text-connection");
const slider_containers = document.querySelectorAll(".slider-container");
const encoder0 = document.querySelector("#encoder0");
const encoder1 = document.querySelector("#encoder1");
const encoder2 = document.querySelector("#encoder2");
const clk = document.querySelector("#clk");

let device;
let messageCharacteristic;

const ble_payload = new Uint8Array(2);
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
        }
    }
    updateValue();
    slider.addEventListener('input', updateValue);
    slider.addEventListener('change', updateValueOnRelease);
})

function update_sliders_ble(bytes) {
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

function handleReceivedData(event) {
    console.log("mensagem recebida\n");
    const value = event.target.value;   
    let bytes = [];

    for (let i = 0; i < value.byteLength; i++) {
        bytes.push(value.getUint8(i));
    }

    console.log("Received bytes:", bytes);

    update_sliders_ble(bytes);
}


async function connectDevice() {
    console.log("connecting...");
    if (device.gatt.connected) return;

    const server = await device.gatt.connect();
    const service = await server.getPrimaryService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

    messageCharacteristic = await service.getCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8");
    messageCharacteristic.addEventListener("characteristicvaluechanged", handleReceivedData);
    disconnectBTN.classList.remove("hide");
    connectBTN.classList.add("hide");
    console.log("connected");

    //change connection status icon
    bt_connection_icon.textContent = "bluetooth_connected";
    text_connection.textContent = "Device connected";
}

async function disconnectDevice() {
    // Verificar se o dispositivo foi definido e se ele está conectado
    if (device && device.gatt.connected) {
        console.log("Disconnecting...");
        
        device.gatt.disconnect();
        
        console.log("BLE device disconnected");
        device = null;
        
        if (connectBTN) {
            connectBTN.textContent = "connect";
        }

    }
    disconnectBTN.classList.add("hide");
    connectBTN.classList.remove("hide");

    //change connection status icon
    bt_connection_icon.textContent = "bluetooth_disabled";
    text_connection.textContent = "No device connected";
}

async function requestDevice() {
    const options = {
    acceptAllDevices: true,
    optionalServices: ["4fafc201-1fb5-459e-8fcc-c5c9c331914b"],
    };
    device = await navigator.bluetooth.requestDevice(options);

    //device.addEventListener("gattserverdisconnected", connectDevice);
}

async function startMonitoring() {
    await messageCharacteristic.startNotifications();
    //heartUI.classList.remove("pause-animation");
}



async function sendData(data) {
    if (!messageCharacteristic) {
        console.error("Characteristic not intialized.");
        return;
    }
    
    //const dataBuffer = new TextEncoder().encode(data); 

    //console.log(`Sending data: ${data}`);

    // ignore message if data is currently being sent
    if (busy_ble) {
        return;
    }

    busy_ble = true;
    await messageCharacteristic.writeValue(data);
    busy_ble = false;

    console.log("Data" + data + "sent successfully.");
}


async function init() {
    if (!navigator.bluetooth) return errorTxt.classList.remove("hide");

    if (!device) await requestDevice();
    //change connection status on page
    connectBTN.textContent = "connecting...";
    bt_connection_icon.textContent = "bluetooth_searching";
    text_connection.textContent = "Connecting"
    await connectDevice();
    await startMonitoring();
    //appUI.classList.remove("hide"); 
}

connectBTN.addEventListener("click", init);
disconnectBTN.addEventListener("click", disconnectDevice);
sendBTN.addEventListener("click", sendData.bind(null, 5));