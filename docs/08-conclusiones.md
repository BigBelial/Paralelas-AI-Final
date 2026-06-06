# 08 — Conclusiones

## Resumen de resultados

> Cerrar con 2-3 frases ancladas en las cifras de
> [03-paralelizacion.md](03-paralelizacion.md) y
> [07-analisis-comparativo.md](07-analisis-comparativo.md): el mejor speedup
> obtenido en Alfa-Beta a `depth=12` fue **\_\_×** con **8** hilos (eficiencia
> **\_\_**); MCTS escaló mejor por ser casi *embarrassingly parallel*. En la nube,
> pasar de 1 a 3 réplicas mejoró el throughput de **\_\_** a **\_\_** req/s.

El motor de Kalah(6,4) quedó funcional con Minimax + poda Alfa-Beta (el algoritmo
exigido) y un segundo motor MCTS, ambos paralelizados con OpenMP, expuestos por un
backend FastAPI y consumidos por un frontend web, todo separado en tres
contenedores y orquestado con Kubernetes en local y en la nube.

## Limitaciones encontradas

- **Pérdida de podas en root parallelism**: al no compartir las cotas α/β entre
  hilos, la versión paralela de Alfa-Beta explora más nodos que la secuencial, lo
  que aleja el speedup del ideal. Es la limitación central y está cuantificada en
  [03-paralelizacion.md](03-paralelizacion.md).
- **Exploración redundante en MCTS**: cada hilo construye su propio árbol, así que
  varios redescubren las mismas jugadas buenas; escala bien pero desperdicia parte
  del cómputo.
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
- Enriquecer `/metrics` con contadores Prometheus reales del motor (nodos/podas
  agregados) y un dashboard Grafana.
- Autoescalado del backend basado en latencia p95 además de CPU.

## Lecciones aprendidas

> Una o dos frases por integrante sobre lo aprendido en paralelización (cómo las
> dependencias de datos limitan el speedup), contenedores (separación de
> responsabilidades), Kubernetes (probes, requests/limits, Services) y CI/CD
> (publicación de imágenes y análisis estático automatizados).
