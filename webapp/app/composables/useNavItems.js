import { computed } from 'vue';
import Home01Icon from '@hugeicons/core-free-icons/Home01Icon';
import MessageSquareTextIcon from '@hugeicons/core-free-icons/MessageSquareTextIcon';
import Settings01Icon from '@hugeicons/core-free-icons/Settings01Icon';
import WifiFullSignalIcon from '@hugeicons/core-free-icons/WifiFullSignalIcon';

// Pages of the device UI, with the current one marked (the sidebar on larger screens and the bottom navigation on phones show the same items)
export const useNavItems = () => {
    const { t } = useI18n();
    const route = useRoute();
    const localePath = useLocalePath();

    // Items, top to bottom in the sidebar and left to right in the bottom navigation (plain paths: the shell adds the language prefix)
    return computed(() => [
        { id: 'home', label: t('nav.home.label'), description: t('nav.home.description'), icon: Home01Icon, to: '/' },
        { id: 'message', label: t('nav.message.label'), description: t('nav.message.description'), icon: MessageSquareTextIcon, to: '/message' },
        { id: 'settings', label: t('nav.settings.label'), description: t('nav.settings.description'), icon: Settings01Icon, to: '/settings' },
        { id: 'wifi', label: t('nav.wifi.label'), description: t('nav.wifi.description'), icon: WifiFullSignalIcon, to: '/wifi' }
    ].map(item => ({ ...item, active: route.path === localePath(item.to) })));
};
