import CpuIcon from '@hugeicons/core-free-icons/CpuIcon';
import SparklesIcon from '@hugeicons/core-free-icons/SparklesIcon';

// Name, logo and pages of the app, read by the layout and the pages of the gizmo-common layer
export default defineAppConfig({
    gizmo: {
        name: 'Holiday Strip',

        // mDNS name of the firmware (http://holiday-strip.local)
        hostname: 'holiday-strip',

        logo: '/logo.svg',
        homePath: '/strip',

        // Pages of the dashboard, top to bottom in the sidebar and left to right in the bottom navigation, in the order of every OpenGizmo app: the strip, then Device, then WiFi ('wifi' is the page of the layer)
        nav: [
            { id: 'strip', labelKey: 'nav.strip.label', descriptionKey: 'nav.strip.description', icon: SparklesIcon, to: '/strip' },
            { id: 'device', labelKey: 'nav.device.label', descriptionKey: 'nav.device.description', icon: CpuIcon, to: '/device' },
            'wifi'
        ]
    }
});