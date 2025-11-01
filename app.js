// Register the service worker
if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('/sw.js')
    .then((reg) => console.log('SW registered', reg))
    .catch((error) => console.error('SW registration failed', error));
}