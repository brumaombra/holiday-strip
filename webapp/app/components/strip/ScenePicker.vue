<script setup>
import { computed, ref } from 'vue';
import DrinkIcon from '@hugeicons/core-free-icons/DrinkIcon';
import FireIcon from '@hugeicons/core-free-icons/FireIcon';
import FireworksIcon from '@hugeicons/core-free-icons/FireworksIcon';
import GiftIcon from '@hugeicons/core-free-icons/GiftIcon';
import HourglassIcon from '@hugeicons/core-free-icons/HourglassIcon';
import ClockIcon from '@hugeicons/core-free-icons/Clock01Icon';
import CandyCaneIcon from '@hugeicons/core-free-icons/CandyCaneIcon';
import LightbulbIcon from '@hugeicons/core-free-icons/LightbulbIcon';
import MessageSquareTextIcon from '@hugeicons/core-free-icons/MessageSquareTextIcon';
import PartyIcon from '@hugeicons/core-free-icons/PartyIcon';
import PineTreeIcon from '@hugeicons/core-free-icons/PineTreeIcon';
import RepeatIcon from '@hugeicons/core-free-icons/RepeatIcon';
import SnowflakeIcon from '@hugeicons/core-free-icons/SnowflakeIcon';
import SparklesIcon from '@hugeicons/core-free-icons/SparklesIcon';
import StarIcon from '@hugeicons/core-free-icons/StarIcon';
import { ListRow } from '@brumaombra/ui-vintage/list';
import { OptionPickerDialog } from '@brumaombra/ui-vintage/option-picker-dialog';
import { getSceneName } from '~/composables/useHolidays.js';

// Props: the scenes of the holiday playing ({ id, name } in the order of the device), the chosen scene (-1 rotates them all) and the one on screen now
const props = defineProps({
    scenes: { type: Array, required: true },
    modelValue: { type: Number, default: -1 },
    currentScene: { type: Number, default: -1 }
});

// Emits
const emits = defineEmits(['update:modelValue']);

const { t, te } = useI18n();

// Icon of each scene by id
const sceneIcons = {
    greeting: MessageSquareTextIcon,
    snow: SnowflakeIcon,
    trees: PineTreeIcon,
    sleigh: StarIcon,
    fireplace: FireIcon,
    garland: LightbulbIcon,
    gifts: GiftIcon,
    'candy-cane': CandyCaneIcon,
    'christmas-countdown': HourglassIcon,
    countdown: ClockIcon,
    confetti: PartyIcon,
    fireworks: FireworksIcon,
    champagne: DrinkIcon,
    sparklers: SparklesIcon,
    message: MessageSquareTextIcon
};

// Options to pick from: rotate them all, then one per scene
const options = computed(() => [
    { value: -1, label: t('pages.strip.rotateAll'), description: t('pages.strip.rotateAllDescription'), icon: RepeatIcon },
    ...props.scenes.map((scene, index) => ({ value: index, label: getSceneName({ scene, t, te }), description: '', icon: sceneIcons[scene.id] || StarIcon }))
]);

// Whether the scene picker is open
const isPickerOpen = ref(false);

// The chosen option (the rotation when the chosen scene isn't in the list)
const selectedOption = computed(() => options.value.find(option => option.value === props.modelValue) || options.value[0]);

// The scene on screen now, under the row (none while the display is off)
const onScreenText = computed(() => {
    const option = options.value.find(item => item.value === props.currentScene && item.value >= 0);
    return option ? t('pages.strip.onScreenScene', { scene: option.label }) : '';
});

// Pick a scene (a tap on the chosen one changes nothing)
const handleSelect = value => {
    if (value !== props.modelValue) emits('update:modelValue', value);
};
</script>

<template>
    <!-- The scene of the strip: a row of the holiday group, with the chosen scene and the one on screen, opening the picker -->
    <ListRow :label="t('pages.strip.sceneLabel')" :value="selectedOption.label" :description="onScreenText" :icon="selectedOption.icon" action data-testid="scene-row" @click="isPickerOpen = true" />

    <!-- The scenes to pick from: the rotation, then one per scene -->
    <OptionPickerDialog v-model:open="isPickerOpen" :title="t('pages.strip.sceneLabel')" :description="t('pages.strip.scenesDescription')" :options="options" :model-value="props.modelValue" @update:model-value="handleSelect" />
</template>