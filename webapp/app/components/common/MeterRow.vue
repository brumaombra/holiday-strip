<script setup>
import { HugeiconsIcon } from '@hugeicons/vue';
import { Progress } from '@brumaombra/ui-vintage/progress';

// Props: the label and the current value on the first line, a bar under them, and what the ends of the bar mean
const props = defineProps({
    label: { type: String, required: true },
    icon: { type: [Object, Array], default: null },
    value: { type: Number, required: true },
    max: { type: Number, required: true },
    valueText: { type: String, default: '' },
    startLabel: { type: String, default: '' },
    endLabel: { type: String, default: '' }
});
</script>

<template>
    <!-- Row of a group: icon, label and value on the first line, a bar full width under them -->
    <li data-slot="list-row" class="px-4 py-3 md:px-5">
        <div class="flex min-h-7 items-center gap-2 md:gap-3">
            <!-- Icon -->
            <HugeiconsIcon v-if="props.icon" :icon="props.icon" class="size-4 shrink-0 text-muted-foreground" />

            <!-- Label -->
            <span class="min-w-0 flex-1 text-[0.8125rem] font-semibold text-foreground md:text-sm">{{ props.label }}</span>

            <!-- Current value -->
            <span class="shrink-0 font-mono text-[0.6875rem] text-muted-foreground md:text-xs">{{ props.valueText || props.value }}</span>
        </div>

        <!-- Bar -->
        <Progress :model-value="props.value" :max="props.max" :aria-label="props.label" class="mt-2" />

        <!-- What the ends of the bar mean -->
        <div v-if="props.startLabel || props.endLabel" class="mt-2 flex items-center justify-between text-[0.6875rem] text-muted-foreground md:text-xs">
            <span>{{ props.startLabel }}</span>
            <span>{{ props.endLabel }}</span>
        </div>
    </li>
</template>
