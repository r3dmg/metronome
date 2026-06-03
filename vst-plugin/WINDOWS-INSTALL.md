# Metronome — установка на Windows (FL Studio)

## Скачать готовый плагин

1. Откройте: **https://github.com/r3dmg/metronome/actions/workflows/build-windows-vst.yml**
2. Выберите последний успешный запуск (зелёная галочка).
3. Внизу страницы в **Artifacts** скачайте **Metronome-Windows-VST3** (файл `Metronome-Windows-x64.zip`).
4. Распакуйте архив — внутри папка **`Metronome.vst3`**.

Если сборки ещё нет: нажмите **Run workflow** → **Run workflow**, подождите ~5–10 минут и обновите страницу.

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
