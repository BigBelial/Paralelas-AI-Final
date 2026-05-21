# 08 — Conclusiones

## Resumen de resultados

> **Pendiente**: 2-3 párrafos con los resultados cuantitativos clave (mejor speedup obtenido, mejor configuración nube, qué algoritmo escaló mejor).

## Limitaciones encontradas

> **Pendiente** — ejemplos posibles que se llenarán durante la implementación:
> - Saturación del bus de memoria al subir hilos en Alfa-Beta.
> - Costo de sincronización de contadores en *tree parallelization* de MCTS.
> - Latencia de red entre pods al delegar del backend al motor.
> - Límites del clúster gestionado (cuotas, tamaño de nodo).

## Retos durante el desarrollo y cómo se resolvieron

> **Pendiente**: registrar incidencias reales del proceso (por ejemplo: depuración de podas perdidas en root parallelism, configuración de CORS preflight, *image pull policy* en el clúster).

## Recomendaciones de mejoras futuras

> **Pendiente** — ejemplos posibles:
> - Implementar *transposition table* compartida entre hilos para Alfa-Beta.
> - Tablebases de finales para Kalah(6,4).
> - Métricas Prometheus enriquecidas y dashboard Grafana.
> - Autoscaling horizontal del backend basado en latencia p95.

## Lecciones aprendidas

> **Pendiente**: 1 o 2 párrafos por integrante del grupo sobre lo aprendido en paralelización, contenedores, Kubernetes y CI/CD.
