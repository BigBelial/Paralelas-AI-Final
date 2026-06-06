# 08 — Conclusiones

## Resumen de resultados

El mejor speedup obtenido en Alfa-Beta a `depth=12` fue **2.20×** con **8** hilos
(eficiencia **0.28**), medido sobre la suite en una VM de 8 vCPU; el detalle está
en [03-paralelizacion.md](03-paralelizacion.md). La eficiencia decreciente se
explica por la pérdida de podas del root parallelism (la versión paralela explora
un 49 % más de nodos que la secuencial) sumada al límite de núcleos físicos.

> Pendiente de la nube: cerrar con el throughput observado al pasar de 1 a 3
> réplicas del backend (ver [07-analisis-comparativo.md](07-analisis-comparativo.md)).

El motor de Kalah(6,4) quedó funcional con Minimax + poda Alfa-Beta (el algoritmo
exigido) paralelizado con OpenMP, expuesto por un backend FastAPI y consumido por
un frontend web, todo separado en tres contenedores y orquestado con Kubernetes en
local y en la nube.

## Limitaciones encontradas

- **Pérdida de podas en root parallelism**: al no compartir las cotas α/β entre
  hilos, la versión paralela de Alfa-Beta explora más nodos que la secuencial, lo
  que aleja el speedup del ideal. Es la limitación central y está cuantificada en
  [03-paralelizacion.md](03-paralelizacion.md).
- **Latencia de red backend→motor**: separar el motor en su propio contenedor
  (requisito del proyecto) añade un salto de red por jugada frente a enlazarlo en
  el mismo proceso; es el costo de la separación pedagógica.

## Retos durante el desarrollo y cómo se resolvieron

- **Turno extra en Alfa-Beta**: rompe la alternancia clásica de Minimax. Se
  resolvió razonando siempre desde la perspectiva del lado de la raíz y leyendo el
  turno real del estado, en vez de asumir que maximizador y minimizador alternan.
- **Verificar que la poda no cambia el resultado**: se programó un Minimax sin poda
  dentro de las pruebas y se comparó su valor contra el de Alfa-Beta a varias
  profundidades; el test falla si difieren.
- **CORS y preflight**: el navegador envía un `OPTIONS` antes del `POST` con JSON.
  Hubo que declarar orígenes, métodos y cabeceras explícitos (sin `*`) para que el
  preflight pasara.
- **URL del backend según el entorno**: el cliente llama al backend directamente,
  pero el puerto cambia (8000 en Compose, 30080 en NodePort, `/api` tras Ingress).
  Se resolvió externalizando la URL a `frontend/public/config.js`.
- **Tag inmutable de imágenes**: GHCR no admite mayúsculas en el nombre del
  repositorio; el workflow pasa el prefijo a minúsculas y usa el SHA del commit
  como tag en lugar de `latest`.

## Recomendaciones de mejoras futuras

- Implementar **YBWC** o **PVS** para recuperar parte de las podas perdidas
  explorando el primer hijo en secuencial antes de abrir el paralelismo.
- Añadir una **tabla de transposición** compartida para no reexplorar posiciones
  repetidas.
- Exponer `/metrics` (que ya agrega nodos/podas del motor) en un dashboard
  Grafana y añadir histogramas de latencia por jugada.
- Autoescalado del backend basado en latencia p95 además de CPU.

## Lecciones aprendidas

> Una o dos frases por integrante sobre lo aprendido en paralelización (cómo las
> dependencias de datos limitan el speedup), contenedores (separación de
> responsabilidades), Kubernetes (probes, requests/limits, Services) y CI/CD
> (publicación de imágenes y análisis estático automatizados).
