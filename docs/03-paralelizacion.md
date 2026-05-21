# 03 — Paralelización con OpenMP

## Estrategia para Alfa-Beta

> **Pendiente de elegir**: indicar cuál de las tres estrategias se implementó y justificar.

Opciones consideradas:

1. **Root parallelism** — reparte los movimientos del nodo raíz entre hilos con `#pragma omp parallel for`, cada uno ejecuta una búsqueda Alfa-Beta secuencial sobre su sub-árbol. Es la estrategia más simple y la base mínima.
2. **Young Brothers Wait Concept (YBWC)** — explora el primer hijo secuencialmente para obtener una buena cota $\beta$, luego paraleliza los hermanos restantes. Reduce la pérdida de podas del esquema anterior.
3. **Principal Variation Splitting (PVS)** — la búsqueda de la variante principal se hace secuencial; los nodos de re-search se distribuyen en paralelo.

**Costo de sincronización** de las cotas $\alpha$ y $\beta$ entre hilos: pendiente de documentar con números reales.

## Estrategia para MCTS

> **Pendiente de elegir**: indicar cuál de las tres estrategias se implementó y justificar.

Opciones consideradas:

1. **Leaf parallelization** — al llegar a una hoja, se lanzan $k$ rollouts independientes en paralelo y se promedia el resultado antes de retropropagar.
2. **Root parallelization** — cada hilo construye su propio árbol MCTS sobre la misma posición raíz; al finalizar se combinan las estadísticas de los hijos del nodo raíz.
3. **Tree parallelization** — todos los hilos comparten un único árbol; los contadores $w$ y $N$ se protegen con `#pragma omp atomic` o locks, complementado con *virtual loss*.

## Instrumentación obligatoria

Cada motor puede ejecutarse en **modo benchmark** sin pasar por el backend HTTP, leyendo un conjunto de posiciones de prueba comunes.

### Métricas comunes

- Tiempo de pared ($T(p)$): `omp_get_wtime` o `std::chrono::steady_clock`.
- **Speedup**: $S(p) = T(1) / T(p)$.
- **Eficiencia**: $E(p) = S(p) / p$.

### Específicas de Alfa-Beta

Por cada par (profundidad, hilos):
- Número de nodos explorados.
- Número de podas Alfa-Beta efectuadas.

### Específicas de MCTS

Por cada par (simulaciones, hilos):
- Número total de rollouts ejecutados.
- Profundidad promedio del árbol construido.
- Tasa de coincidencia con la jugada óptima de Alfa-Beta.

## Barrido experimental

Mediciones para $p \in \{1, 2, 4, 8\}$ hilos. Para Alfa-Beta: al menos dos profundidades (e.g. `depth=8` y `depth=12`). Para MCTS: al menos dos presupuestos (e.g. `simulations=10000` y `simulations=100000`).

### Tablas T(p), S(p), E(p)

Alfa-Beta, `depth=12`:

| p | T(p) [s] | S(p) | E(p) |
|---|---|---|---|
| 1 | — | — | — |
| 2 | — | — | — |
| 4 | — | — | — |
| 8 | — | — | — |

MCTS, `simulations=100000`:

| p | T(p) [s] | S(p) | E(p) |
|---|---|---|---|
| 1 | — | — | — |
| 2 | — | — | — |
| 4 | — | — | — |
| 8 | — | — | — |

> **Pendiente de llenar** con datos reales una vez se ejecuten los benchmarks.

## Gráficas de speedup

> Insertar diagrama Mermaid o tabla ASCII con la curva de speedup vs. hilos para cada algoritmo.

## Comparación directa entre algoritmos

A presupuesto de cómputo equivalente (mismo tiempo de pared por jugada): qué algoritmo encuentra el mejor movimiento con más frecuencia y cuál escala mejor con el número de hilos.

## Herramientas de profiling usadas

Al menos dos de las siguientes, con capturas o registros como evidencia:

- `perf stat` y `perf record` — contadores de hardware.
- `htop` / `top` — ocupación efectiva de núcleos.
- `time`, `/usr/bin/time -v` — tiempo y RSS máximo.
- `valgrind --tool=callgrind` — opcional.

### Ejemplos de invocación

```bash
# Alfa-Beta secuencial
OMP_NUM_THREADS=1 ./mancala_bench --algo alphabeta --depth 12 \
    --positions tests/suite.txt

# Alfa-Beta paralelo con perf stat
OMP_NUM_THREADS=8 perf stat -e cycles,instructions,cache-misses \
    ./mancala_bench --algo alphabeta --depth 12 \
    --positions tests/suite.txt

# MCTS paralelo
OMP_NUM_THREADS=8 ./mancala_bench --algo mcts --simulations 100000 \
    --positions tests/suite.txt
```

> **Pendiente**: capturas de `perf`, `htop` y `time` durante la ejecución.
