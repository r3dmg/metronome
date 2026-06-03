# Metronome — установка на Windows (FL Studio)

## Скачать готовый плагин (проще всего)

**Прямая ссылка (после сборки на GitHub):**

https://github.com/r3dmg/metronome/releases/download/metronome-windows/Metronome-Windows-x64.zip

Распакуйте архив — внутри папка **`Metronome.vst3`**.

Альтернатива: [Actions](https://github.com/r3dmg/metronome/actions/workflows/build-windows-vst.yml) → последний зелёный запуск → **Artifacts** → `Metronome-Windows-VST3`.

## Установка

1. Скопируйте папку **`Metronome.vst3`** в:

   ```
   C:\Program Files\Common Files\VST3\
   ```

2. FL Studio → **Options → Manage plugins** → **Find more plugins** / **Refresh**.
3. Включите **Metronome**, добавьте на дорожку, нажмите **Play**.

## Содержимое архива

- Встроенные WAV (kick, snare, hi-hat, crash, tom)
- Отсчёт **3 → 2 → 1** (переключатель **Countdown 3s**)
- Синхронизация BPM с FL (**Sync FL tempo**)
