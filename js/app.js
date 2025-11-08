const connectBTN = document.querySelector(".connect");
const disconnectBTN = document.querySelector(".disconnect");
const sendBTN = document.querySelector(".send");
const errorTxt = document.querySelector(".error");
const bt_connection_icon = document.querySelector(".icon-connection");
const text_connection = document.querySelector(".text-connection");
const slider_containers = document.querySelectorAll(".slider-container");

let device;
let messageCharacteristic;
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
    }
    updateValue();
    slider.addEventListener('input', updateValue);
})

function handleReceivedData(event) {
    console.log("mensagem recebida\n");
    const value = event.target.value;   
    let bytes = [];

    for (let i = 0; i < value.byteLength; i++) {
        bytes.push(value.getUint8(i));
    }

    console.log("Received bytes:", bytes);
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

    } else if (device) {
        console.log("Device already disconnected.");
    } else {
        console.log("No device selected for disconnection.");
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
    
    const dataBuffer = new TextEncoder().encode(data); 

    console.log(`Sending data: ${data}`);

    await messageCharacteristic.writeValue(dataBuffer);

    console.log("Data sent successfully.");
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