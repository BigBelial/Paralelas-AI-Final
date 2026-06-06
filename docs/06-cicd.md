# 06 — CI/CD y Calidad de Código

## Workflow en `.github/workflows/`

Todo el pipeline está descrito en YAML en [`ci.yml`](../.github/workflows/ci.yml),
con cuatro jobs:

| Job | Qué hace |
|---|---|
| `build-and-test-motor` | Compila el motor C++ con CMake + OpenMP, corre los tests unitarios (`ctest`) y un *smoke* de los dos algoritmos. |
| `test-backend` | Instala dependencias y corre `pytest` del backend. |
| `docker-images` | Construye las 3 imágenes y, en push a `main`/`master`, las publica en GHCR con tag inmutable. |
| `sonarqube` | Ejecuta el escáner de SonarCloud declarado en YAML. |

## Pipeline

```mermaid
flowchart LR
    push[Push / PR a main] --> motor[Build & test motor<br/>CMake + OpenMP + ctest]
    push --> back[pytest backend]
    motor --> imgs[Build imágenes Docker]
    back --> imgs
    imgs -->|solo en push| ghcr[Publicar en GHCR<br/>tag = SHA]
    motor --> sonar[SonarCloud Scan]
    back --> sonar
    sonar --> gate{Quality Gate}
    gate -->|pasa| ok[Build verde]
    gate -->|falla| fail[Build rojo]
```

## Publicación de imágenes

En pull requests solo se **construyen** las imágenes (validación); en push a la
rama principal se **publican** a GHCR. El tag es el SHA del commit
(`ghcr.io/<owner>/<repo>/mancala-<componente>:<sha>`), nunca `latest`, para que el
despliegue sea reproducible. El prefijo del repositorio se pasa a minúsculas
porque GHCR no admite mayúsculas en el nombre.

## Integración con SonarCloud

Declarada **en YAML** con `sonarsource/sonarqube-scan-action@v2`, **no** como
plugin del marketplace (requisito explícito de la rúbrica). La configuración del
proyecto vive en [`sonar-project.properties`](../sonar-project.properties) en la
raíz: claves del proyecto, rutas de fuentes (`motor/src`, `backend/app`,
`frontend/public`), rutas de tests y exclusiones.

El job expone el token a nivel de job y **se salta solo si no hay `SONAR_TOKEN`**,
para que el CI quede en verde mientras no esté configurado; en cuanto se cargue el
secret, el escaneo corre automáticamente. Si no se define `SONAR_HOST_URL`, se usa
`https://sonarcloud.io` por defecto, así que para SonarCloud basta con configurar
un único secret.

```yaml
env:
  SONAR_TOKEN: ${{ secrets.SONAR_TOKEN }}
  SONAR_HOST_URL: ${{ secrets.SONAR_HOST_URL || 'https://sonarcloud.io' }}
steps:
  - uses: actions/checkout@v4
    with:
      fetch-depth: 0
  - name: SonarQube Scan
    if: ${{ env.SONAR_TOKEN != '' }}
    uses: sonarsource/sonarqube-scan-action@v2
```

Para activarlo:

1. Crear el proyecto en SonarCloud y generar un token.
2. Añadir `SONAR_TOKEN` en *Settings → Secrets and variables → Actions*
   (y `SONAR_HOST_URL` solo si NO se usa SonarCloud, p. ej. un SonarQube propio).
3. Ajustar `sonar.projectKey` / `sonar.organization` en `sonar-project.properties`
   a los valores reales del grupo.

## Evidencia

> Adjuntar aquí las capturas de:
> - Una ejecución del workflow en verde (los 4 jobs) en la pestaña Actions.
> - El Quality Gate de SonarCloud (pasando) y el resumen de issues/cobertura.
