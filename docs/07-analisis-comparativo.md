# 07 — Análisis Comparativo: Local vs. Nube

## Configuración del experimento

- **Local**: una sola instancia del backend en máquina física o VM, variando `OMP_NUM_THREADS ∈ {1, 2, 4, 8}`. Únicamente motor Alfa-Beta a profundidad fija.
- **Nube**: el mismo backend en Kubernetes con dos configuraciones de réplicas, $r \in \{1, 3\}$. `OMP_NUM_THREADS` fijo a un valor moderado (e.g. `2`) y se varía $r$.
- **Carga sintética**: peticiones concurrentes con `wrk`, `k6` o `ab`. Cada petición envía un estado de tablero y una profundidad fija.

Imagen del motor y profundidad de búsqueda se mantienen idénticas en ambos entornos.

## Métricas reportadas

- **Latencia por petición**: percentiles $p_{50}$ y $p_{95}$.
- **Throughput**: peticiones/segundo sostenidas.

## Resultados — Local (variando hilos)

Una sola instancia del backend, misma posición y profundidad, variando
`OMP_NUM_THREADS`. **Llenar con la salida real de la prueba de carga.**

| `OMP_NUM_THREADS` | p50 [ms] | p95 [ms] | Throughput [req/s] |
|---|---|---|---|
| 1 | _(carga)_ | _(carga)_ | _(carga)_ |
| 2 | _(carga)_ | _(carga)_ | _(carga)_ |
| 4 | _(carga)_ | _(carga)_ | _(carga)_ |
| 8 | _(carga)_ | _(carga)_ | _(carga)_ |

## Resultados — Nube (variando réplicas, `OMP_NUM_THREADS=2`)

Mismo backend en Kubernetes, hilos fijos en 2, variando réplicas. **Llenar con la
salida real medida en el clúster del grupo.**

| Réplicas $r$ | p50 [ms] | p95 [ms] | Throughput [req/s] |
|---|---|---|---|
| 1 | _(carga)_ | _(carga)_ | _(carga)_ |
| 3 | _(carga)_ | _(carga)_ | _(carga)_ |

## Observación cualitativa

> Cerrar con una o dos frases ancladas en los números: en general, escalar
> **verticalmente** (más hilos por pod) conviene mientras el motor siga ganando
> speedup con más hilos (eficiencia alta) y haya CPU disponible en el nodo;
> escalar **horizontalmente** (más réplicas) conviene cuando el cuello de botella
> es la concurrencia de peticiones y la eficiencia por hilo ya cae. Confirmar la
> afirmación con los p95 y el throughput de las tablas de arriba.

## Comando de carga utilizado

Los scripts están en [`deploy/local/loadtest/`](../deploy/local/loadtest/):

```bash
# Opción A — wrk (script post_move.lua)
wrk -t4 -c50 -d30s -s deploy/local/loadtest/post_move.lua http://<host>:8000/move

# Opción B — k6 (reporta p50/p95 directamente)
BASE=http://<host>:8000 k6 run deploy/local/loadtest/move.js
```

> Adjuntar las capturas de salida de la herramienta de carga (wrk o k6) en local
> y en la nube como evidencia experimental.
