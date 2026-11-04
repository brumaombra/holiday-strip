<script setup>
import { computed } from 'vue';
import FireIcon from '@hugeicons/core-free-icons/FireIcon';
import FireworksIcon from '@hugeicons/core-free-icons/FireworksIcon';
import GiftIcon from '@hugeicons/core-free-icons/GiftIcon';
import CandyCaneIcon from '@hugeicons/core-free-icons/CandyCaneIcon';
import LightbulbIcon from '@hugeicons/core-free-icons/LightbulbIcon';
import MessageSquareTextIcon from '@hugeicons/core-free-icons/MessageSquareTextIcon';
import PineTreeIcon from '@hugeicons/core-free-icons/PineTreeIcon';
import RepeatIcon from '@hugeicons/core-free-icons/RepeatIcon';
import SnowflakeIcon from '@hugeicons/core-free-icons/SnowflakeIcon';
import StarIcon from '@hugeicons/core-free-icons/StarIcon';
import Tick02Icon from '@hugeicons/core-free-icons/Tick02Icon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Badge } from '@brumaombra/ui-vintage/badge';

// Props: the scene names in the order of the device, the chosen scene (-1 rotates them all) and the one on screen now
const props = defineProps({
    scenes: { type: Array, required: true },
    modelValue: { type: Number, default: -1 },
    currentScene: { type: Number, default: -1 }
});

// Emits
const emits = defineEmits(['update:modelValue']);

const { t, te } = useI18n();

// Icon of each scene, in the order of the device
const sceneIcons = [MessageSquareTextIcon, SnowflakeIcon, PineTreeIcon, StarIcon, FireIcon, LightbulbIcon, GiftIcon, CandyCaneIcon, FireworksIcon];

// Name of a scene in the language of the app (the name sent by the device is the fallback for a scene this app doesn't know)
const getSceneName = ({ name, index }) => {
    const key = `pages.home.sceneNames.${index}`;
    return te(key) ? t(key) : name;
};

// Options to pick from: rotate them all, then one per scene
const options = computed(() => [
    { value: -1, label: t('pages.home.rotateAll'), description: t('pages.home.rotateAllDescription'), icon: RepeatIcon },
    ...props.scenes.map((name, index) => ({ value: index, label: getSceneName({ name, index }), description: '', icon: sceneIcons[index] || StarIcon }))
]);

// Pick a scene (a tap on the chosen one changes nothing)
const handleSelect = value => {
    if (value !== props.modelValue) emits('update:modelValue', value);
};
</script>

<template>
    <ul class="divide-y divide-border overflow-hidden rounded border border-border bg-card">
        <li v-for="option in options" :key="option.value">
            <button type="button" class="flex min-h-12 w-full cursor-pointer items-center gap-3 px-3 py-2 text-left transition-colors hover:bg-accent" :class="{ 'bg-accent': option.value === props.modelValue }" :aria-pressed="option.value === props.modelValue" @click="handleSelect(option.value)">
                <!-- Icon of the scene -->
                <HugeiconsIcon :icon="option.icon" class="size-5 shrink-0 text-muted-foreground" />

                <!-- Name -->
                <span class="min-w-0 flex-1">
                    <span class="block text-sm font-semibold">{{ option.label }}</span>
                    <span v-if="option.description" class="mt-0.5 block text-xs text-muted-foreground">{{ option.description }}</span>
                </span>

                <!-- The scene on screen now -->
                <Badge v-if="option.value === props.currentScene" :text="t('pages.home.onScreen')" color="green" size="sm" pulse />

                <!-- The chosen scene -->
                <HugeiconsIcon v-if="option.value === props.modelValue" :icon="Tick02Icon" class="size-4 shrink-0 text-primary" />
            </button>
        </li>
    </ul>
</template>
