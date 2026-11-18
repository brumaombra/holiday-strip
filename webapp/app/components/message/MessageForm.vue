<script setup>
import { computed, ref } from 'vue';
import Alert02Icon from '@hugeicons/core-free-icons/Alert02Icon';
import CheckmarkCircle02Icon from '@hugeicons/core-free-icons/CheckmarkCircle02Icon';
import MessageSquareTextIcon from '@hugeicons/core-free-icons/MessageSquareTextIcon';
import SaveIcon from '@hugeicons/core-free-icons/SaveIcon';
import { HugeiconsIcon } from '@hugeicons/vue';
import { Alert, AlertDescription, AlertTitle } from '@brumaombra/ui-vintage/alert';
import { Button } from '@brumaombra/ui-vintage/button';
import { ListGroup, ListRow } from '@brumaombra/ui-vintage/list';
import { showMessageToast } from '@brumaombra/ui-vintage/message-toast';
import { Textarea } from '@brumaombra/ui-vintage/textarea';
import { applyDeviceSettings } from '~/composables/useDeviceState.js';
import { describeCharacter, findUnsupportedCharacters, getMessageByteLength, MESSAGE_HEART, MESSAGE_LETTERS_AND_DIGITS, MESSAGE_PUNCTUATION } from '~/composables/useMessage.js';
import { getHolidayName } from '~/composables/useHolidays.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Props: the index of the holiday whose message is edited
const props = defineProps({
    holidayIndex: { type: Number, required: true }
});

const { t, te } = useI18n();
const globalStore = useGlobalStore();
const isSaving = ref(false);

// Reactive device state
const device = computed(() => globalStore.value.device);

// The holiday of this message, and the id of its field
const holiday = computed(() => device.value.holidays[props.holidayIndex]);
const fieldId = computed(() => `message-${holiday.value.id}`);

// The message of the device for this holiday
const deviceMessage = computed(() => device.value.messages[props.holidayIndex] ?? '');

// The text being edited (it starts as the message of the device, and is only sent with Save)
const draft = ref(deviceMessage.value);

// Length of the draft in bytes, against what the device accepts
const draftLength = computed(() => getMessageByteLength(draft.value));
const isTooLong = computed(() => draftLength.value > device.value.maxMessageLength);

// Characters of the draft the matrix can't draw
const unsupportedCharacters = computed(() => findUnsupportedCharacters(draft.value));
const unsupportedList = computed(() => unsupportedCharacters.value.map(describeCharacter).join(' '));

// Whether the draft differs from the message on the device (which is stored in capitals)
const hasChanges = computed(() => draft.value.toUpperCase() !== deviceMessage.value);

// Save the message on the device
const handleSave = async () => {
    if (isTooLong.value || isSaving.value) return;

    try {
        isSaving.value = true;
        const messages = device.value.holidays.map((_, index) => index === props.holidayIndex ? draft.value : null);
        const isSaved = await applyDeviceSettings({ messages });

        // Show the message as the device stored it, in capitals
        if (isSaved) {
            draft.value = deviceMessage.value;
            showMessageToast({ type: 'success', icon: CheckmarkCircle02Icon, message: t('pages.message.saveSuccess') });
        }
    } finally {
        isSaving.value = false;
    }
};
</script>

<template>
    <form class="space-y-6" @submit.prevent="handleSave">
        <!-- The message, with the characters the matrix can draw under the group -->
        <ListGroup :title="getHolidayName({ holiday, t, te })" :hint="t('pages.message.allowedHint', { letters: MESSAGE_LETTERS_AND_DIGITS, punctuation: MESSAGE_PUNCTUATION, heart: MESSAGE_HEART })">
            <ListRow :label="t('pages.message.label')" :icon="MessageSquareTextIcon" :label-for="fieldId">
                <!-- Message -->
                <Textarea :id="fieldId" v-model="draft" rows="3" autocapitalize="characters" spellcheck="false" :placeholder="t('pages.message.placeholder')" :aria-invalid="isTooLong || undefined" />

                <!-- Counter against the limit of the device -->
                <p class="mt-2 text-right text-xs tabular-nums" :class="isTooLong ? 'font-semibold text-destructive' : 'text-muted-foreground'" :data-testid="`message-counter-${holiday.id}`">
                    {{ t('pages.message.counter', { length: draftLength, max: device.maxMessageLength }) }}
                </p>
            </ListRow>
        </ListGroup>

        <!-- Characters the font can't draw -->
        <Alert v-if="unsupportedCharacters.length" color="yellow">
            <HugeiconsIcon :icon="Alert02Icon" />
            <AlertTitle>{{ t('pages.message.unsupportedTitle') }}</AlertTitle>
            <AlertDescription>{{ t('pages.message.unsupportedDescription', { characters: unsupportedList }) }}</AlertDescription>
        </Alert>

        <!-- Save button -->
        <Button variant="primary" type="submit" class="w-full" :data-testid="`message-save-${holiday.id}`" :loading="isSaving" :disabled="isTooLong || !hasChanges">
            <HugeiconsIcon :icon="SaveIcon" class="size-4" />
            {{ t('pages.message.saveAction') }}
        </Button>
    </form>
</template>
