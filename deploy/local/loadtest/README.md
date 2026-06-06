# Prueba de carga (análisis comparativo local vs. nube)

Scripts para generar las métricas de latencia (p50/p95) y throughput que pide
[docs/07-analisis-comparativo.md](../../../docs/07-analisis-comparativo.md).
Apuntan al endpoint `POST /move` del backend.

Usa la **misma posición, profundidad e hilos** en ambos entornos para que la
comparación sea honesta. Solo cambia la capa de orquestación (hilos en local,
réplicas en la nube).

## Con wrk

```bash
wrk -t4 -c50 -d30s -s post_move.lua http://<host>:8000/move
```

## Con k6 (reporta p50/p95 directamente)

```bash
BASE=http://<host>:8000 k6 run move.js
```

## Barrido sugerido

- **Local**: una sola instancia, variando `OMP_NUM_THREADS ∈ {1, 2, 4, 8}`.
- **Nube**: `OMP_NUM_THREADS=2` fijo, variando réplicas `r ∈ {1, 3}`.

Pega los percentiles y el throughput en las tablas de `docs/07`.
