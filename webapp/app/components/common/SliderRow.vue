<script setup>
import { computed } from 'vue';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Slider } from '@brumaombra/ui-vintage/slider';

// Props: the label and the current value on the first line, the slider under them (it stays inline, so dragging it while watching the device is one gesture)
const props = defineProps({
    id: { type: String, default: undefined },
    label: { type: String, required: true },
    icon: { type: [Object, Array], default: null },
    modelValue: { type: Number, required: true },
    min: { type: Number, default: 0 },
    max: { type: Number, default: 100 },
    step: { type: Number, default: 1 },
    valueText: { type: String, default: '' }
});

// Emits
const emits = defineEmits(['update:modelValue']);

// Slider value as the list the slider wants
const sliderValue = computed(() => [props.modelValue]);

// Pass the first slider value back to the parent
const handleUpdateModelValue = value => {
    emits('update:modelValue', value?.[0] ?? props.min);
};
</script>

<template>
    <!-- Row of a group: icon, label and value on the first line, the slider full width under them -->
    <li data-slot="list-row" class="px-4 py-3 md:px-5">
        <div class="flex min-h-7 items-center gap-2 md:gap-3">
            <!-- Icon -->
            <HugeiconsIcon v-if="props.icon" :icon="props.icon" class="size-4 shrink-0 text-muted-foreground" />

            <!-- Label -->
            <label :for="props.id" class="min-w-0 flex-1 text-[0.8125rem] font-semibold text-foreground md:text-sm">{{ props.label }}</label>

            <!-- Current value -->
            <span class="shrink-0 font-mono text-[0.6875rem] text-muted-foreground md:text-xs" data-testid="slider-row-value">{{ props.valueText || props.modelValue }}</span>
        </div>

        <!-- Slider -->
        <Slider :id="props.id" :model-value="sliderValue" :min="props.min" :max="props.max" :step="props.step" :thumb-label="props.label" @update:model-value="handleUpdateModelValue" />
    </li>
</template>
