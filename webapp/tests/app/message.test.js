import { describe, expect, it } from 'vitest';
import { describeCharacter, findUnsupportedCharacters, getMessageByteLength, isCharacterSupported } from '~/composables/useMessage.js';

// Characters the matrix font can draw: A-Z, 0-9, space, ! ' * , - . : ? and the heart (<)
describe('isCharacterSupported', () => {
    // Letters, digits and the punctuation of the font
    it('accepts the characters of the font', () => {
        for (const character of [...'ABCXYZ0189 !\'*,-.:?<']) {
            expect(isCharacterSupported(character), character).toBe(true);
        }
    });

    // The device shows lowercase in capitals, so it is not a problem
    it('accepts lowercase letters', () => {
        for (const character of [...'abcxyz']) {
            expect(isCharacterSupported(character), character).toBe(true);
        }
    });

    // Anything else has no glyph
    it('refuses what the font has no glyph for', () => {
        for (const character of [...'àèéùç€@#%&()+/=;_"\n\tß']) {
            expect(isCharacterSupported(character), JSON.stringify(character)).toBe(false);
        }
    });

    // The dotless i and the long s become capital I and S in JavaScript, but not on the device
    it('refuses letters that only look like a-z', () => {
        expect(isCharacterSupported('ı')).toBe(false);
        expect(isCharacterSupported('ſ')).toBe(false);
    });
});

// The warning under the message field
describe('findUnsupportedCharacters', () => {
    // A message the matrix can draw has nothing to warn about
    it('finds nothing in a message the font can draw', () => {
        expect(findUnsupportedCharacters('BUON NATALE * MERRY CHRISTMAS <3')).toEqual([]);
        expect(findUnsupportedCharacters('')).toEqual([]);
    });

    // Each wrong character is listed once, in order
    it('lists each unsupported character once, in order', () => {
        expect(findUnsupportedCharacters('Buon Natale è bello, più è meglio @ 100%')).toEqual(['è', 'ù', '@', '%']);
    });

    // A line break from the text area is not a space for the matrix
    it('flags line breaks', () => {
        expect(findUnsupportedCharacters('CIAO\nMONDO')).toEqual(['\n']);
    });

    // An emoji is one character for the user, not two halves
    it('lists an emoji as one character', () => {
        expect(findUnsupportedCharacters('CIAO 🎄')).toEqual(['🎄']);
    });

    // A missing message is an empty one
    it('accepts a missing message', () => {
        expect(findUnsupportedCharacters(undefined)).toEqual([]);
        expect(findUnsupportedCharacters(null)).toEqual([]);
    });
});

// The counter counts bytes, like the device
describe('getMessageByteLength', () => {
    // ASCII is one byte per character
    it('counts one byte for each plain character', () => {
        expect(getMessageByteLength('MERRY CHRISTMAS')).toBe(15);
        expect(getMessageByteLength('')).toBe(0);
    });

    // An accented letter takes two bytes, so it counts twice against the limit
    it('counts the bytes of characters outside ASCII', () => {
        expect(getMessageByteLength('è')).toBe(2);
        expect(getMessageByteLength('🎄')).toBe(4);
    });
});

// Characters in the warning must be readable
describe('describeCharacter', () => {
    // A line break would be invisible in the warning
    it('writes the invisible characters', () => {
        expect(describeCharacter('\n')).toBe('\\n');
        expect(describeCharacter('\t')).toBe('\\t');
        expect(describeCharacter('è')).toBe('è');
    });
});
