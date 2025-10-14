const connectBTN = document.querySelector(".connect");
let device;
// Register the service worker
/*if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('/sw.js')
    .then((reg) => console.log('SW registered', reg))
    .catch((error) => console.error('SW registration failed', error));
}*/

async function requestDevice() {
    const options = {
    acceptAllDevices: true,
    optionalServices: ["4fafc201-1fb5-459e-8fcc-c5c9c331914b"],
    };
    device = await navigator.bluetooth.requestDevice(options);

    device.addEventListener("gattserverdisconnected", connectDevice);
}
async function init() {
    console.log("init function")
    if (!navigator.bluetooth) return errorTxt.classList.remove("hide");

    if (!device) await requestDevice();
    connectBTN.textContent = "connecting...";
    await connectDevice();
    await startMonitoring();
    //appUI.classList.remove("hide"); 
}
connectBTN.addEventListener("click", init);