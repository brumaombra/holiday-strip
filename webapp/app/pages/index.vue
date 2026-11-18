<script setup>
import { onMounted } from 'vue';
import { LoadingStateCard } from '@brumaombra/ui-vintage/loading-state-card';

const { t } = useI18n();
const localePath = useLocalePath();

// Open the strip page, the first page of the app (like every OpenGizmo app, the root only leads to it)
onMounted(() => {
    navigateTo(localePath('/strip'), { replace: true });
});

// Page metadata
definePageMeta({
    layout: 'dashboard'
});
</script>

<template>
    <div class="mx-auto w-full max-w-2xl">
        <LoadingStateCard :title="t('common.redirectingStrip')" />
    </div>
</template>