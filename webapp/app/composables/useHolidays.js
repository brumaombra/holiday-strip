// Get the name of a holiday in the language of the app (the name sent by the device is the fallback for a holiday this app doesn't know)
export const getHolidayName = ({ holiday, t, te }) => {
    const key = `pages.strip.holidayNames.${holiday.id}`;
    return te(key) ? t(key) : holiday.name;
};
