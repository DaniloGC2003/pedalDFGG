// install service worker
self.addEventListener('install', event => {
  console.log('Service Worker installed.');
});

// activate service worker
self.addEventListener('activate', event => {
  console.log('Service Worker activated.');
});

self.addEventListener('fetch', event => {
  console.log('fetch event', event);
});
 