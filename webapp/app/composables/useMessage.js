// Characters the matrix font can draw (lowercase letters are shown in capitals), and the heart
export const MESSAGE_LETTERS_AND_DIGITS = 'A-Z 0-9';
export const MESSAGE_PUNCTUATION = '! \' * , - . : ?';
export const MESSAGE_HEART = '<';
const ALLOWED_CHARACTERS = new Set([...'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !\'*,-.:?<']);

// Check if the matrix font can draw a character (only the plain a-z are folded to capitals, like the device does)
export const isCharacterSupported = character => {
    const folded = /^[a-z]$/.test(character) ? character.toUpperCase() : character;
    return ALLOWED_CHARACTERS.has(folded);
};

// Find the characters of a message the matrix can't draw, each one once, in order of appearance
export const findUnsupportedCharacters = message => {
    return [...new Set([...String(message ?? '')].filter(character => !isCharacterSupported(character)))];
};

// Get the length of a message in bytes, like the device counts it (a character outside ASCII takes more than one)
export const getMessageByteLength = message => {
    return new TextEncoder().encode(String(message ?? '')).length;
};

// Show a character in the warning (a space or a line break would be invisible)
export const describeCharacter = character => {
    if (character === '\n') return '\\n';
    if (character === '\t') return '\\t';
    return character;
};
