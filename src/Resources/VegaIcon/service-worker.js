/* Vega PWA service worker.
   A `fetch` handler is mandatory: without it Chrome on Android (and Yandex
   mobile) will not fire `beforeinstallprompt`, so the app cannot be installed. */

const CACHE = "vega-v1";

const APP_SHELL = [
  "./",
  "./Vega.html",
  "./Vega.js",
  "./Vega.wasm",
  "./qtloader.js",
  "./manifest.json",
  "./Resources/VegaIcon/icon-192.png",
  "./Resources/VegaIcon/icon-512.png",
];

self.addEventListener("install", (event) => {
  event.waitUntil(
    caches.open(CACHE).then((cache) =>
      // Don't fail the whole install if one optional asset 404s.
      Promise.allSettled(APP_SHELL.map((url) => cache.add(url)))
    )
  );
  self.skipWaiting();
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
    caches
      .keys()
      .then((keys) =>
        Promise.all(keys.filter((k) => k !== CACHE).map((k) => caches.delete(k)))
      )
      .then(() => self.clients.claim())
  );
});

self.addEventListener("fetch", (event) => {
  const req = event.request;
  if (req.method !== "GET" || new URL(req.url).origin !== self.location.origin) {
    return;
  }
  event.respondWith(
    caches.match(req).then((cached) => {
      const network = fetch(req)
        .then((res) => {
          if (res && res.ok) {
            const copy = res.clone();
            caches.open(CACHE).then((cache) => cache.put(req, copy));
          }
          return res;
        })
        .catch(() => cached);
      return cached || network;
    })
  );
});
