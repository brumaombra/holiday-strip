<script setup>
// Props
const props = defineProps({
    brandName: { type: String, required: true }
});
</script>

<template>
    <div class="flex items-center gap-2 sm:gap-3">
        <!-- Brand mark -->
        <img src="/logo.svg" alt="" aria-hidden="true" class="size-8 sm:size-9">

        <!-- Brand name -->
        <span class="text-base font-bold text-foreground sm:text-lg">{{ props.brandName }}</span>
    </div>
</template>
