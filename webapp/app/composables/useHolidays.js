// Translate a name the device sends, by its id (the English name of the device is the fallback for one this app doesn't know)
const translateName = ({ key, fallback, t, te }) => te(key) ? t(key) : fallback;

// Get the name of a holiday in the language of the app
export const getHolidayName = ({ holiday, t, te }) => translateName({ key: `pages.strip.holidayNames.${holiday.id}`, fallback: holiday.name, t, te });

// Get the name of a scene in the language of the app
export const getSceneName = ({ scene, t, te }) => translateName({ key: `pages.strip.sceneNames.${scene.id}`, fallback: scene.name, t, te });