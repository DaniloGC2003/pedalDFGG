self.addEventListener('install', event => {
  console.log('Service Worker installing.');
});

self.addEventListener('fetch', event => {
  // basic; could add caching at some point
});
