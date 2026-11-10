# Port chip for the [Wokwi](https://wokwi.com) simulator

You can use this chip by adding this Git repository as a dependency in your `diagram.json` file.

```json
{
  ...
  "parts": [
    ...
    {
      "type": "chip-port",
      "id": "chip1",
      "top": 0,
      "left": 0,
      "attrs": {}
    }
    ...
  ],
  "dependencies": { "chip-port": "github:twsimpson/wokwi-chip-por@main" }
}
```