<script setup>
import { onMounted, ref } from 'vue';
import { drawFrame, getCanvasSize } from '~/composables/useFrame.js';
import { getFrame } from '~/composables/useDeviceApi.js';
import { usePolling } from '~/composables/usePolling.js';

const FRAME_REFRESH_MS = 200; // About 5 frames per second
const { t } = useI18n();
const canvas = ref(null);
const { width, height } = getCanvasSize();

// Draw a frame on the canvas (nothing if the canvas is not there yet or the browser can't draw)
const paintFrame = frame => {
    const context = canvas.value?.getContext('2d');
    if (!context) return;
    drawFrame({ context, frame });
};

// Read the frame on the matrix and draw it (a failed read keeps the last frame on screen)
const refreshFrame = async () => {
    const frame = await getFrame();
    if (frame) paintFrame(frame);
};

// Draw the dark LEDs right away, so the matrix is there before the first frame arrives
onMounted(() => paintFrame(null));

// Refresh while the page is visible (the poller stops when the tab is hidden and when the page is left)
usePolling({ callback: refreshFrame, intervalMs: FRAME_REFRESH_MS });
</script>

<template>
    <!-- Live preview of the matrix -->
    <canvas ref="canvas" :width="width" :height="height" role="img" :aria-label="t('pages.home.previewLabel')" class="block h-auto w-full rounded" style="aspect-ratio: 6 / 1" />
</template>
