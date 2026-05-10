self.addEventListener('install', (e) => {
  self.skipWaiting();
});

self.addEventListener('activate', (e) => {
  e.waitUntil(clients.claim());
});

self.addEventListener('fetch', (e) => {
  e.respondWith(
    caches.match(e.request).then((response) => {
      return response || fetch(e.request).then((fetchResponse) => {
        return caches.open('vega-v1').then((cache) => {
          if (e.request.url.startsWith('http')) {
            cache.put(e.request, fetchResponse.clone());
          }
          return fetchResponse;
        });
      });
    })
  );
});