# Mesh Shading for Visibility Culling

## Generelle TODO
- Bounding Box/Sphere intersection fix
- ~~Meshlet generation effizienter machen~~ (Index Buffer mit Tipsy optimiert)
- Vertex Attribute compression

## Culling Ablauf

- Erstes Frame: Alle Meshlets Drawcall
- Immer: View Frustum Culling (fertig, wenn Bounding intersection gefixt ist)
- Meshlet ID in separaten Buffer rendern
- Compute Shader über ID Buffer sammelt die Meshlets, die sichtbar waren
- Diese Meshlets wieder rendern
- Aus Depth Buffer eine Mipmap Pyramide generieren
- Im nächsten Frame: Meshlets aus ID Buffer Drawcall
- Für alle anderen Meshlets: Bounding Box/Sphere minimum Depth berechnen und mit Depth Pyramide vergleichen
- Wenn berechnete Depth < Previous, also potentiell visible: Draw call (Indirect mesh invocation aus compute shader?)

## Evaluierung
Vergleich mit Traditioneller Vertex Pipeline
### Demo Szene
Viel Overdraw, Camera Movement was die Occlusion stark verändert (Work in Progress)
- Ich hätte mir überlegt eine Art Arena mit Säulen vorgestellt, wo die Kamera hinter den Säulen fliegt die immer wieder die Szene verdecken
- Vielleicht verändert sich die Szene nach jeder Säule für pseudo "Animationen", vielleicht nicht sinnvoll und wichtig
### Scripted Benchmark Run
Fixierter Kameraflug durch die Demo Szene, sammelt Timings und andere Statistiken und schreibt sie in eine Datei
### Platformen(optional)
Ich hätte Zugriff auf sowohl Nvidia, AMD und Apple Hardware mit Mesh Shading, und es würde Sinn machen den Unterschied Mesh-Traditionell auf allen Platformed zu vergleichen. Um Apple zu benchmarken, müsste ich ein Metal-Backend implementieren.