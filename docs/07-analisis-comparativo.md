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

| `OMP_NUM_THREADS` | p50 [ms] | p95 [ms] | Throughput [req/s] |
|---|---|---|---|
| 1 | — | — | — |
| 2 | — | — | — |
| 4 | — | — | — |
| 8 | — | — | — |

## Resultados — Nube (variando réplicas, `OMP_NUM_THREADS=2`)

| Réplicas $r$ | p50 [ms] | p95 [ms] | Throughput [req/s] |
|---|---|---|---|
| 1 | — | — | — |
| 3 | — | — | — |

## Observación cualitativa

> **Pendiente** (una o dos frases con números reales): cuándo conviene escalar **verticalmente** (más hilos por pod) y cuándo **horizontalmente** (más réplicas), anclado en los datos obtenidos.

## Comando de carga utilizado

```bash
# Ejemplo con wrk
wrk -t4 -c50 -d30s -s post_move.lua http://<host>:8000/move
```

> **Pendiente**: incluir el script `post_move.lua` y capturas de salida de la herramienta de carga.
