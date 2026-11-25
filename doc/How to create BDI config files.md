# How To Create BDI Config File

Create a JSON config file ending in `_BDI.json` (for example `MyTestMod_BDI.json`)
under `Data\SKSE\Plugins\BehaviorDataInjector`.

Example:

```json
[
    {
        "projectPath": "Actors",
        "type": "kInt",
        "name": "MaxsuTestInt",
        "value": 123
    },
    {
        "projectPath": "actors\\Character",
        "type": "kBool",
        "name": "MaxsuTestBool",
        "value": true
    },
    {
        "projectPath": "Actors\\Horse",
        "type": "kFloat",
        "name": "MaxsuTestFloat",
        "value": 123.12
    },
    {
        "projectPath": "Actors",
        "type": "kEvent",
        "name": "MaxsuTestEvent"
    }
]
```

`projectPath` is relative to `Data\Meshes\`.
Supported types are `kInt`, `kBool`, `kFloat`, and `kEvent`.
`value` is not required for an animation event.

Set `EnableDebugLog` in `BehaviorDataInjector.ini` to enable debug messages.
The original console helper remains available: select an actor and use e.g.
`DGV MaxsuTestInt i`.
