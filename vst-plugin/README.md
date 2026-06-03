# Metronome VST3 / AU

VST3-плагин на основе [веб-метронома](../README.md): клик по долям, барабанные паттерны (Accelonome), Auto-BPM и 5 профилей настроек.

## Совместимость с FL Studio

| Платформа | Формат | Путь после сборки |
|-----------|--------|-------------------|
| **macOS** | VST3, AU | `~/Library/Audio/Plug-Ins/VST3/Metronome.vst3` |
| **Windows** | VST3 | `C:\Program Files\Common Files\VST3\Metronome.vst3` |

В FL Studio: **Options → Manage plugins → Refresh** → найдите **Metronome** и добавьте на Master или отдельную дорожку.

Плагин синхронизируется с транспортом FL Studio. Включите **Sync FL tempo**, чтобы BPM брался из проекта; иначе используется ручной BPM.

## Требования для сборки

- CMake 3.22+
- C++17 компилятор (Xcode на macOS, Visual Studio 2022 на Windows)
- Git (для загрузки JUCE через CMake)

### macOS

```bash
brew install cmake
cd vst-plugin
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Windows (x64, из Developer Command Prompt)

```bat
cd vst-plugin
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

После сборки скопируйте `Metronome.vst3` из `build/MetronomeVST_artefacts/Release/VST3/` в системную папку VST3 (см. таблицу выше).

## Standalone

Та же сборка создаёт standalone-приложение — удобно проверить метроном без DAW:

`build/MetronomeVST_artefacts/Release/Standalone/`

## Функции (как в веб-версии)

- BPM 20–300, размер такта (1–12 / 2,4,8,16)
- Клик с акцентом на первой доле
- Барабаны: паттерны 4/4, 4/5, 4/6, 8/6 (синтез, как fallback в браузере)
- Auto-BPM: шаг, min/max, каждые N тактов или минут, loop / reverse
- 5 профилей (кнопки 1–5), состояние сохраняется в пресете проекта

## Сэмплы и отсчёт

- Ударные **встроены в плагин** (те же WAV, что на [демо-сайте](https://r3dmg.github.io/metronome/)): kick, snare, hi-hat, crash, tom.
- **Countdown 3s** — при старте Play: «3 → 2 → 1» с кликом (как в веб-версии), затем метроном. Переключатель **Countdown 3s** в UI.

## Отличия от веб-версии

- Старт отсчёта и метронома привязаны к **Play** в DAW (не к кнопке Start).
- Auto-BPM при синхронизации с хостом меняет внутренний темп только если **Sync FL tempo** выключен.

## Лицензия JUCE

Сборка тянет [JUCE](https://juce.com/) (GPL / commercial — при публикации плагина учтите лицензию JUCE).
