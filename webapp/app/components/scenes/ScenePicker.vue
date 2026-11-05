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
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';

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
    <!-- The scenes are the content of the group: one row each, the chosen one ticked -->
    <ListGroup :title="t('pages.home.scenesTitle')" :hint="t('pages.home.scenesDescription')">
        <!-- A button row per scene, with the description under the name -->
        <ListRow v-for="option in options" :key="option.value" :label="option.label" :description="option.description" :icon="option.icon" action  data-testid="scene-row" @click="handleSelect(option.value)">
            <template #trailing>
                <!-- The scene on screen now -->
                <Badge v-if="option.value === props.currentScene" :text="t('pages.home.onScreen')" color="green" size="sm" pulse />

                <!-- The chosen scene -->
                <HugeiconsIcon v-if="option.value === props.modelValue" :icon="Tick02Icon" class="size-4 shrink-0 text-primary" data-testid="scene-selected" />
            </template>
        </ListRow>
    </ListGroup>
</template>
