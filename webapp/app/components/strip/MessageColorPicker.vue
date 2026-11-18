<script setup>
import { computed, ref } from 'vue';
import ColorsIcon from '@hugeicons/core-free-icons/ColorsIcon';
import PaintBoardIcon from '@hugeicons/core-free-icons/PaintBoardIcon';
import { ListRow } from '@brumaombra/ui-vintage/list';
import { OptionPickerDialog } from '@brumaombra/ui-vintage/option-picker-dialog';

// Props: the color of the custom message (an index of the colors of the firmware)
const props = defineProps({
    modelValue: { type: Number, default: 0 }
});

// Emits
const emits = defineEmits(['update:modelValue']);

const { t } = useI18n();

// The colors of the firmware, in its order (the last one is the rainbow)
const COLOR_KEYS = ['white', 'warmWhite', 'red', 'green', 'blue', 'gold', 'rainbow'];

// Options to pick from: one per color
const options = computed(() => COLOR_KEYS.map((key, index) => ({
    value: index,
    label: t(`pages.strip.message.colors.${key}`),
    description: '',
    icon: key === 'rainbow' ? ColorsIcon : PaintBoardIcon
})));

// Whether the color picker is open
const isPickerOpen = ref(false);

// The chosen option (white when the color isn't in the list)
const selectedOption = computed(() => options.value.find(option => option.value === props.modelValue) || options.value[0]);

// Pick a color (a tap on the chosen one changes nothing)
const handleSelect = value => {
    if (value !== props.modelValue) emits('update:modelValue', value);
};
</script>

<template>
    <!-- The color of the custom message: a row with the chosen color, opening the picker -->
    <ListRow :label="t('pages.strip.message.colorLabel')" :value="selectedOption.label" :icon="selectedOption.icon" action data-testid="message-color-row" @click="isPickerOpen = true" />

    <!-- The colors to pick from -->
    <OptionPickerDialog v-model:open="isPickerOpen" :title="t('pages.strip.message.colorLabel')" :options="options" :model-value="props.modelValue" @update:model-value="handleSelect" />
</template>