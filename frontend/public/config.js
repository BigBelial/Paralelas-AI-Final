// Configuración del cliente que se puede cambiar sin recompilar la imagen.
//
// Define la URL base del backend al que el navegador hace las peticiones.
// Si se deja como cadena vacía, app.js usa http://<host-actual>:8000, que es
// lo correcto para Docker Compose (el backend se publica en el puerto 8000).
//
// Cámbiala según el entorno:
//   - Docker Compose:            "" (vacío)
//   - Kubernetes local NodePort: "http://<IP-del-nodo>:30080"
//   - Nube detrás de un Ingress: "/api"
window.MANCALA_API_BASE = "";
