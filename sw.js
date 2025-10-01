const staticCacheName = 'site-static';
const assets = [
  '/',
  '/index.html',
  '/css/styles.css',
  '/js/app.js',
  '/img/images/mrbeast.webp',
  "https://fonts.googleapis.com/icon?family=Material+Icons"
];

// install service worker
self.addEventListener('install', event => {
  event.waitUntil(
    caches.open(staticCacheName).then(cache => {
      console.log('caching shell assets');
      cache.addAll(assets);
    })
  );
});

// activate service worker
self.addEventListener('activate', event => {
  console.log('Service Worker activated.');
});

self.addEventListener('fetch', event => {
  console.log('fetch event', event);
});
 