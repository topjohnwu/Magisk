package com.topjohnwu.magisk.terminal

/**
 * Implementation of wcwidth(3) for Unicode 15.
 *
 * Implementation from https://github.com/jquast/wcwidth but we return 0 for unprintable characters.
 *
 * IMPORTANT:
 * Must be kept in sync with the following:
 * https://github.com/termux/wcwidth
 * https://github.com/termux/libandroid-support
 * https://github.com/termux/termux-packages/tree/master/packages/libandroid-support
 */
object WcWidth {

    // From https://github.com/jquast/wcwidth/blob/master/wcwidth/table_zero.py
    // from https://github.com/jquast/wcwidth/pull/64
    // at commit 1b9b6585b0080ea5cb88dc9815796505724793fe (2022-12-16):
    private val ZERO_WIDTH = arrayOf(
        intArrayOf(0x00300, 0x0036f),  // Combining Grave Accent  ..Combining Latin Small Le
        intArrayOf(0x00483, 0x00489),  // Combining Cyrillic Titlo..Combining Cyrillic Milli
        intArrayOf(0x00591, 0x005bd),  // Hebrew Accent Etnahta   ..Hebrew Point Meteg
        intArrayOf(0x005bf, 0x005bf),  // Hebrew Point Rafe       ..Hebrew Point Rafe
        intArrayOf(0x005c1, 0x005c2),  // Hebrew Point Shin Dot   ..Hebrew Point Sin Dot
        intArrayOf(0x005c4, 0x005c5),  // Hebrew Mark Upper Dot   ..Hebrew Mark Lower Dot
        intArrayOf(0x005c7, 0x005c7),  // Hebrew Point Qamats Qata..Hebrew Point Qamats Qata
        intArrayOf(0x00610, 0x0061a),  // Arabic Sign Sallallahou ..Arabic Small Kasra
        intArrayOf(0x0064b, 0x0065f),  // Arabic Fathatan         ..Arabic Wavy Hamza Below
        intArrayOf(0x00670, 0x00670),  // Arabic Letter Superscrip..Arabic Letter Superscrip
        intArrayOf(0x006d6, 0x006dc),  // Arabic Small High Ligatu..Arabic Small High Seen
        intArrayOf(0x006df, 0x006e4),  // Arabic Small High Rounde..Arabic Small High Madda
        intArrayOf(0x006e7, 0x006e8),  // Arabic Small High Yeh   ..Arabic Small High Noon
        intArrayOf(0x006ea, 0x006ed),  // Arabic Empty Centre Low ..Arabic Small Low Meem
        intArrayOf(0x00711, 0x00711),  // Syriac Letter Superscrip..Syriac Letter Superscrip
        intArrayOf(0x00730, 0x0074a),  // Syriac Pthaha Above     ..Syriac Barrekh
        intArrayOf(0x007a6, 0x007b0),  // Thaana Abafili          ..Thaana Sukun
        intArrayOf(0x007eb, 0x007f3),  // Nko Combining Short High..Nko Combining Double Dot
        intArrayOf(0x007fd, 0x007fd),  // Nko Dantayalan          ..Nko Dantayalan
        intArrayOf(0x00816, 0x00819),  // Samaritan Mark In       ..Samaritan Mark Dagesh
        intArrayOf(0x0081b, 0x00823),  // Samaritan Mark Epentheti..Samaritan Vowel Sign A
        intArrayOf(0x00825, 0x00827),  // Samaritan Vowel Sign Sho..Samaritan Vowel Sign U
        intArrayOf(0x00829, 0x0082d),  // Samaritan Vowel Sign Lon..Samaritan Mark Nequdaa
        intArrayOf(0x00859, 0x0085b),  // Mandaic Affrication Mark..Mandaic Gemination Mark
        intArrayOf(0x00898, 0x0089f),  // Arabic Small High Word A..Arabic Half Madda Over M
        intArrayOf(0x008ca, 0x008e1),  // Arabic Small High Farsi ..Arabic Small High Sign S
        intArrayOf(0x008e3, 0x00902),  // Arabic Turned Damma Belo..Devanagari Sign Anusvara
        intArrayOf(0x0093a, 0x0093a),  // Devanagari Vowel Sign Oe..Devanagari Vowel Sign Oe
        intArrayOf(0x0093c, 0x0093c),  // Devanagari Sign Nukta   ..Devanagari Sign Nukta
        intArrayOf(0x00941, 0x00948),  // Devanagari Vowel Sign U ..Devanagari Vowel Sign Ai
        intArrayOf(0x0094d, 0x0094d),  // Devanagari Sign Virama  ..Devanagari Sign Virama
        intArrayOf(0x00951, 0x00957),  // Devanagari Stress Sign U..Devanagari Vowel Sign Uu
        intArrayOf(0x00962, 0x00963),  // Devanagari Vowel Sign Vo..Devanagari Vowel Sign Vo
        intArrayOf(0x00981, 0x00981),  // Bengali Sign Candrabindu..Bengali Sign Candrabindu
        intArrayOf(0x009bc, 0x009bc),  // Bengali Sign Nukta      ..Bengali Sign Nukta
        intArrayOf(0x009c1, 0x009c4),  // Bengali Vowel Sign U    ..Bengali Vowel Sign Vocal
        intArrayOf(0x009cd, 0x009cd),  // Bengali Sign Virama     ..Bengali Sign Virama
        intArrayOf(0x009e2, 0x009e3),  // Bengali Vowel Sign Vocal..Bengali Vowel Sign Vocal
        intArrayOf(0x009fe, 0x009fe),  // Bengali Sandhi Mark     ..Bengali Sandhi Mark
        intArrayOf(0x00a01, 0x00a02),  // Gurmukhi Sign Adak Bindi..Gurmukhi Sign Bindi
        intArrayOf(0x00a3c, 0x00a3c),  // Gurmukhi Sign Nukta     ..Gurmukhi Sign Nukta
        intArrayOf(0x00a41, 0x00a42),  // Gurmukhi Vowel Sign U   ..Gurmukhi Vowel Sign Uu
        intArrayOf(0x00a47, 0x00a48),  // Gurmukhi Vowel Sign Ee  ..Gurmukhi Vowel Sign Ai
        intArrayOf(0x00a4b, 0x00a4d),  // Gurmukhi Vowel Sign Oo  ..Gurmukhi Sign Virama
        intArrayOf(0x00a51, 0x00a51),  // Gurmukhi Sign Udaat     ..Gurmukhi Sign Udaat
        intArrayOf(0x00a70, 0x00a71),  // Gurmukhi Tippi          ..Gurmukhi Addak
        intArrayOf(0x00a75, 0x00a75),  // Gurmukhi Sign Yakash    ..Gurmukhi Sign Yakash
        intArrayOf(0x00a81, 0x00a82),  // Gujarati Sign Candrabind..Gujarati Sign Anusvara
        intArrayOf(0x00abc, 0x00abc),  // Gujarati Sign Nukta     ..Gujarati Sign Nukta
        intArrayOf(0x00ac1, 0x00ac5),  // Gujarati Vowel Sign U   ..Gujarati Vowel Sign Cand
        intArrayOf(0x00ac7, 0x00ac8),  // Gujarati Vowel Sign E   ..Gujarati Vowel Sign Ai
        intArrayOf(0x00acd, 0x00acd),  // Gujarati Sign Virama    ..Gujarati Sign Virama
        intArrayOf(0x00ae2, 0x00ae3),  // Gujarati Vowel Sign Voca..Gujarati Vowel Sign Voca
        intArrayOf(0x00afa, 0x00aff),  // Gujarati Sign Sukun     ..Gujarati Sign Two-circle
        intArrayOf(0x00b01, 0x00b01),  // Oriya Sign Candrabindu  ..Oriya Sign Candrabindu
        intArrayOf(0x00b3c, 0x00b3c),  // Oriya Sign Nukta        ..Oriya Sign Nukta
        intArrayOf(0x00b3f, 0x00b3f),  // Oriya Vowel Sign I      ..Oriya Vowel Sign I
        intArrayOf(0x00b41, 0x00b44),  // Oriya Vowel Sign U      ..Oriya Vowel Sign Vocalic
        intArrayOf(0x00b4d, 0x00b4d),  // Oriya Sign Virama       ..Oriya Sign Virama
        intArrayOf(0x00b55, 0x00b56),  // Oriya Sign Overline     ..Oriya Ai Length Mark
        intArrayOf(0x00b62, 0x00b63),  // Oriya Vowel Sign Vocalic..Oriya Vowel Sign Vocalic
        intArrayOf(0x00b82, 0x00b82),  // Tamil Sign Anusvara     ..Tamil Sign Anusvara
        intArrayOf(0x00bc0, 0x00bc0),  // Tamil Vowel Sign Ii     ..Tamil Vowel Sign Ii
        intArrayOf(0x00bcd, 0x00bcd),  // Tamil Sign Virama       ..Tamil Sign Virama
        intArrayOf(0x00c00, 0x00c00),  // Telugu Sign Combining Ca..Telugu Sign Combining Ca
        intArrayOf(0x00c04, 0x00c04),  // Telugu Sign Combining An..Telugu Sign Combining An
        intArrayOf(0x00c3c, 0x00c3c),  // Telugu Sign Nukta       ..Telugu Sign Nukta
        intArrayOf(0x00c3e, 0x00c40),  // Telugu Vowel Sign Aa    ..Telugu Vowel Sign Ii
        intArrayOf(0x00c46, 0x00c48),  // Telugu Vowel Sign E     ..Telugu Vowel Sign Ai
        intArrayOf(0x00c4a, 0x00c4d),  // Telugu Vowel Sign O     ..Telugu Sign Virama
        intArrayOf(0x00c55, 0x00c56),  // Telugu Length Mark      ..Telugu Ai Length Mark
        intArrayOf(0x00c62, 0x00c63),  // Telugu Vowel Sign Vocali..Telugu Vowel Sign Vocali
        intArrayOf(0x00c81, 0x00c81),  // Kannada Sign Candrabindu..Kannada Sign Candrabindu
        intArrayOf(0x00cbc, 0x00cbc),  // Kannada Sign Nukta      ..Kannada Sign Nukta
        intArrayOf(0x00cbf, 0x00cbf),  // Kannada Vowel Sign I    ..Kannada Vowel Sign I
        intArrayOf(0x00cc6, 0x00cc6),  // Kannada Vowel Sign E    ..Kannada Vowel Sign E
        intArrayOf(0x00ccc, 0x00ccd),  // Kannada Vowel Sign Au   ..Kannada Sign Virama
        intArrayOf(0x00ce2, 0x00ce3),  // Kannada Vowel Sign Vocal..Kannada Vowel Sign Vocal
        intArrayOf(0x00d00, 0x00d01),  // Malayalam Sign Combining..Malayalam Sign Candrabin
        intArrayOf(0x00d3b, 0x00d3c),  // Malayalam Sign Vertical ..Malayalam Sign Circular
        intArrayOf(0x00d41, 0x00d44),  // Malayalam Vowel Sign U  ..Malayalam Vowel Sign Voc
        intArrayOf(0x00d4d, 0x00d4d),  // Malayalam Sign Virama   ..Malayalam Sign Virama
        intArrayOf(0x00d62, 0x00d63),  // Malayalam Vowel Sign Voc..Malayalam Vowel Sign Voc
        intArrayOf(0x00d81, 0x00d81),  // Sinhala Sign Candrabindu..Sinhala Sign Candrabindu
        intArrayOf(0x00dca, 0x00dca),  // Sinhala Sign Al-lakuna  ..Sinhala Sign Al-lakuna
        intArrayOf(0x00dd2, 0x00dd4),  // Sinhala Vowel Sign Ketti..Sinhala Vowel Sign Ketti
        intArrayOf(0x00dd6, 0x00dd6),  // Sinhala Vowel Sign Diga ..Sinhala Vowel Sign Diga
        intArrayOf(0x00e31, 0x00e31),  // Thai Character Mai Han-a..Thai Character Mai Han-a
        intArrayOf(0x00e34, 0x00e3a),  // Thai Character Sara I   ..Thai Character Phinthu
        intArrayOf(0x00e47, 0x00e4e),  // Thai Character Maitaikhu..Thai Character Yamakkan
        intArrayOf(0x00eb1, 0x00eb1),  // Lao Vowel Sign Mai Kan  ..Lao Vowel Sign Mai Kan
        intArrayOf(0x00eb4, 0x00ebc),  // Lao Vowel Sign I        ..Lao Semivowel Sign Lo
        intArrayOf(0x00ec8, 0x00ece),  // Lao Tone Mai Ek         ..(nil)
        intArrayOf(0x00f18, 0x00f19),  // Tibetan Astrological Sig..Tibetan Astrological Sig
        intArrayOf(0x00f35, 0x00f35),  // Tibetan Mark Ngas Bzung ..Tibetan Mark Ngas Bzung
        intArrayOf(0x00f37, 0x00f37),  // Tibetan Mark Ngas Bzung ..Tibetan Mark Ngas Bzung
        intArrayOf(0x00f39, 0x00f39),  // Tibetan Mark Tsa -phru  ..Tibetan Mark Tsa -phru
        intArrayOf(0x00f71, 0x00f7e),  // Tibetan Vowel Sign Aa   ..Tibetan Sign Rjes Su Nga
        intArrayOf(0x00f80, 0x00f84),  // Tibetan Vowel Sign Rever..Tibetan Mark Halanta
        intArrayOf(0x00f86, 0x00f87),  // Tibetan Sign Lci Rtags  ..Tibetan Sign Yang Rtags
        intArrayOf(0x00f8d, 0x00f97),  // Tibetan Subjoined Sign L..Tibetan Subjoined Letter
        intArrayOf(0x00f99, 0x00fbc),  // Tibetan Subjoined Letter..Tibetan Subjoined Letter
        intArrayOf(0x00fc6, 0x00fc6),  // Tibetan Symbol Padma Gda..Tibetan Symbol Padma Gda
        intArrayOf(0x0102d, 0x01030),  // Myanmar Vowel Sign I    ..Myanmar Vowel Sign Uu
        intArrayOf(0x01032, 0x01037),  // Myanmar Vowel Sign Ai   ..Myanmar Sign Dot Below
        intArrayOf(0x01039, 0x0103a),  // Myanmar Sign Virama     ..Myanmar Sign Asat
        intArrayOf(0x0103d, 0x0103e),  // Myanmar Consonant Sign M..Myanmar Consonant Sign M
        intArrayOf(0x01058, 0x01059),  // Myanmar Vowel Sign Vocal..Myanmar Vowel Sign Vocal
        intArrayOf(0x0105e, 0x01060),  // Myanmar Consonant Sign M..Myanmar Consonant Sign M
        intArrayOf(0x01071, 0x01074),  // Myanmar Vowel Sign Geba ..Myanmar Vowel Sign Kayah
        intArrayOf(0x01082, 0x01082),  // Myanmar Consonant Sign S..Myanmar Consonant Sign S
        intArrayOf(0x01085, 0x01086),  // Myanmar Vowel Sign Shan ..Myanmar Vowel Sign Shan
        intArrayOf(0x0108d, 0x0108d),  // Myanmar Sign Shan Counci..Myanmar Sign Shan Counci
        intArrayOf(0x0109d, 0x0109d),  // Myanmar Vowel Sign Aiton..Myanmar Vowel Sign Aiton
        intArrayOf(0x0135d, 0x0135f),  // Ethiopic Combining Gemin..Ethiopic Combining Gemin
        intArrayOf(0x01712, 0x01714),  // Tagalog Vowel Sign I    ..Tagalog Sign Virama
        intArrayOf(0x01732, 0x01733),  // Hanunoo Vowel Sign I    ..Hanunoo Vowel Sign U
        intArrayOf(0x01752, 0x01753),  // Buhid Vowel Sign I      ..Buhid Vowel Sign U
        intArrayOf(0x01772, 0x01773),  // Tagbanwa Vowel Sign I   ..Tagbanwa Vowel Sign U
        intArrayOf(0x017b4, 0x017b5),  // Khmer Vowel Inherent Aq ..Khmer Vowel Inherent Aa
        intArrayOf(0x017b7, 0x017bd),  // Khmer Vowel Sign I      ..Khmer Vowel Sign Ua
        intArrayOf(0x017c6, 0x017c6),  // Khmer Sign Nikahit      ..Khmer Sign Nikahit
        intArrayOf(0x017c9, 0x017d3),  // Khmer Sign Muusikatoan  ..Khmer Sign Bathamasat
        intArrayOf(0x017dd, 0x017dd),  // Khmer Sign Atthacan     ..Khmer Sign Atthacan
        intArrayOf(0x0180b, 0x0180d),  // Mongolian Free Variation..Mongolian Free Variation
        intArrayOf(0x0180f, 0x0180f),  // Mongolian Free Variation..Mongolian Free Variation
        intArrayOf(0x01885, 0x01886),  // Mongolian Letter Ali Gal..Mongolian Letter Ali Gal
        intArrayOf(0x018a9, 0x018a9),  // Mongolian Letter Ali Gal..Mongolian Letter Ali Gal
        intArrayOf(0x01920, 0x01922),  // Limbu Vowel Sign A      ..Limbu Vowel Sign U
        intArrayOf(0x01927, 0x01928),  // Limbu Vowel Sign E      ..Limbu Vowel Sign O
        intArrayOf(0x01932, 0x01932),  // Limbu Small Letter Anusv..Limbu Small Letter Anusv
        intArrayOf(0x01939, 0x0193b),  // Limbu Sign Mukphreng    ..Limbu Sign Sa-i
        intArrayOf(0x01a17, 0x01a18),  // Buginese Vowel Sign I   ..Buginese Vowel Sign U
        intArrayOf(0x01a1b, 0x01a1b),  // Buginese Vowel Sign Ae  ..Buginese Vowel Sign Ae
        intArrayOf(0x01a56, 0x01a56),  // Tai Tham Consonant Sign ..Tai Tham Consonant Sign
        intArrayOf(0x01a58, 0x01a5e),  // Tai Tham Sign Mai Kang L..Tai Tham Consonant Sign
        intArrayOf(0x01a60, 0x01a60),  // Tai Tham Sign Sakot     ..Tai Tham Sign Sakot
        intArrayOf(0x01a62, 0x01a62),  // Tai Tham Vowel Sign Mai ..Tai Tham Vowel Sign Mai
        intArrayOf(0x01a65, 0x01a6c),  // Tai Tham Vowel Sign I   ..Tai Tham Vowel Sign Oa B
        intArrayOf(0x01a73, 0x01a7c),  // Tai Tham Vowel Sign Oa A..Tai Tham Sign Khuen-lue
        intArrayOf(0x01a7f, 0x01a7f),  // Tai Tham Combining Crypt..Tai Tham Combining Crypt
        intArrayOf(0x01ab0, 0x01ace),  // Combining Doubled Circum..Combining Latin Small Le
        intArrayOf(0x01b00, 0x01b03),  // Balinese Sign Ulu Ricem ..Balinese Sign Surang
        intArrayOf(0x01b34, 0x01b34),  // Balinese Sign Rerekan   ..Balinese Sign Rerekan
        intArrayOf(0x01b36, 0x01b3a),  // Balinese Vowel Sign Ulu ..Balinese Vowel Sign Ra R
        intArrayOf(0x01b3c, 0x01b3c),  // Balinese Vowel Sign La L..Balinese Vowel Sign La L
        intArrayOf(0x01b42, 0x01b42),  // Balinese Vowel Sign Pepe..Balinese Vowel Sign Pepe
        intArrayOf(0x01b6b, 0x01b73),  // Balinese Musical Symbol ..Balinese Musical Symbol
        intArrayOf(0x01b80, 0x01b81),  // Sundanese Sign Panyecek ..Sundanese Sign Panglayar
        intArrayOf(0x01ba2, 0x01ba5),  // Sundanese Consonant Sign..Sundanese Vowel Sign Pan
        intArrayOf(0x01ba8, 0x01ba9),  // Sundanese Vowel Sign Pam..Sundanese Vowel Sign Pan
        intArrayOf(0x01bab, 0x01bad),  // Sundanese Sign Virama   ..Sundanese Consonant Sign
        intArrayOf(0x01be6, 0x01be6),  // Batak Sign Tompi        ..Batak Sign Tompi
        intArrayOf(0x01be8, 0x01be9),  // Batak Vowel Sign Pakpak ..Batak Vowel Sign Ee
        intArrayOf(0x01bed, 0x01bed),  // Batak Vowel Sign Karo O ..Batak Vowel Sign Karo O
        intArrayOf(0x01bef, 0x01bf1),  // Batak Vowel Sign U For S..Batak Consonant Sign H
        intArrayOf(0x01c2c, 0x01c33),  // Lepcha Vowel Sign E     ..Lepcha Consonant Sign T
        intArrayOf(0x01c36, 0x01c37),  // Lepcha Sign Ran         ..Lepcha Sign Nukta
        intArrayOf(0x01cd0, 0x01cd2),  // Vedic Tone Karshana     ..Vedic Tone Prenkha
        intArrayOf(0x01cd4, 0x01ce0),  // Vedic Sign Yajurvedic Mi..Vedic Tone Rigvedic Kash
        intArrayOf(0x01ce2, 0x01ce8),  // Vedic Sign Visarga Svari..Vedic Sign Visarga Anuda
        intArrayOf(0x01ced, 0x01ced),  // Vedic Sign Tiryak       ..Vedic Sign Tiryak
        intArrayOf(0x01cf4, 0x01cf4),  // Vedic Tone Candra Above ..Vedic Tone Candra Above
        intArrayOf(0x01cf8, 0x01cf9),  // Vedic Tone Ring Above   ..Vedic Tone Double Ring A
        intArrayOf(0x01dc0, 0x01dff),  // Combining Dotted Grave A..Combining Right Arrowhea
        intArrayOf(0x020d0, 0x020f0),  // Combining Left Harpoon A..Combining Asterisk Above
        intArrayOf(0x02cef, 0x02cf1),  // Coptic Combining Ni Abov..Coptic Combining Spiritu
        intArrayOf(0x02d7f, 0x02d7f),  // Tifinagh Consonant Joine..Tifinagh Consonant Joine
        intArrayOf(0x02de0, 0x02dff),  // Combining Cyrillic Lette..Combining Cyrillic Lette
        intArrayOf(0x0302a, 0x0302d),  // Ideographic Level Tone M..Ideographic Entering Ton
        intArrayOf(0x03099, 0x0309a),  // Combining Katakana-hirag..Combining Katakana-hirag
        intArrayOf(0x0a66f, 0x0a672),  // Combining Cyrillic Vzmet..Combining Cyrillic Thous
        intArrayOf(0x0a674, 0x0a67d),  // Combining Cyrillic Lette..Combining Cyrillic Payer
        intArrayOf(0x0a69e, 0x0a69f),  // Combining Cyrillic Lette..Combining Cyrillic Lette
        intArrayOf(0x0a6f0, 0x0a6f1),  // Bamum Combining Mark Koq..Bamum Combining Mark Tuk
        intArrayOf(0x0a802, 0x0a802),  // Syloti Nagri Sign Dvisva..Syloti Nagri Sign Dvisva
        intArrayOf(0x0a806, 0x0a806),  // Syloti Nagri Sign Hasant..Syloti Nagri Sign Hasant
        intArrayOf(0x0a80b, 0x0a80b),  // Syloti Nagri Sign Anusva..Syloti Nagri Sign Anusva
        intArrayOf(0x0a825, 0x0a826),  // Syloti Nagri Vowel Sign ..Syloti Nagri Vowel Sign
        intArrayOf(0x0a82c, 0x0a82c),  // Syloti Nagri Sign Altern..Syloti Nagri Sign Altern
        intArrayOf(0x0a8c4, 0x0a8c5),  // Saurashtra Sign Virama  ..Saurashtra Sign Candrabi
        intArrayOf(0x0a8e0, 0x0a8f1),  // Combining Devanagari Dig..Combining Devanagari Sig
        intArrayOf(0x0a8ff, 0x0a8ff),  // Devanagari Vowel Sign Ay..Devanagari Vowel Sign Ay
        intArrayOf(0x0a926, 0x0a92d),  // Kayah Li Vowel Ue       ..Kayah Li Tone Calya Plop
        intArrayOf(0x0a947, 0x0a951),  // Rejang Vowel Sign I     ..Rejang Consonant Sign R
        intArrayOf(0x0a980, 0x0a982),  // Javanese Sign Panyangga ..Javanese Sign Layar
        intArrayOf(0x0a9b3, 0x0a9b3),  // Javanese Sign Cecak Telu..Javanese Sign Cecak Telu
        intArrayOf(0x0a9b6, 0x0a9b9),  // Javanese Vowel Sign Wulu..Javanese Vowel Sign Suku
        intArrayOf(0x0a9bc, 0x0a9bd),  // Javanese Vowel Sign Pepe..Javanese Consonant Sign
        intArrayOf(0x0a9e5, 0x0a9e5),  // Myanmar Sign Shan Saw   ..Myanmar Sign Shan Saw
        intArrayOf(0x0aa29, 0x0aa2e),  // Cham Vowel Sign Aa      ..Cham Vowel Sign Oe
        intArrayOf(0x0aa31, 0x0aa32),  // Cham Vowel Sign Au      ..Cham Vowel Sign Ue
        intArrayOf(0x0aa35, 0x0aa36),  // Cham Consonant Sign La  ..Cham Consonant Sign Wa
        intArrayOf(0x0aa43, 0x0aa43),  // Cham Consonant Sign Fina..Cham Consonant Sign Fina
        intArrayOf(0x0aa4c, 0x0aa4c),  // Cham Consonant Sign Fina..Cham Consonant Sign Fina
        intArrayOf(0x0aa7c, 0x0aa7c),  // Myanmar Sign Tai Laing T..Myanmar Sign Tai Laing T
        intArrayOf(0x0aab0, 0x0aab0),  // Tai Viet Mai Kang       ..Tai Viet Mai Kang
        intArrayOf(0x0aab2, 0x0aab4),  // Tai Viet Vowel I        ..Tai Viet Vowel U
        intArrayOf(0x0aab7, 0x0aab8),  // Tai Viet Mai Khit       ..Tai Viet Vowel Ia
        intArrayOf(0x0aabe, 0x0aabf),  // Tai Viet Vowel Am       ..Tai Viet Tone Mai Ek
        intArrayOf(0x0aac1, 0x0aac1),  // Tai Viet Tone Mai Tho   ..Tai Viet Tone Mai Tho
        intArrayOf(0x0aaec, 0x0aaed),  // Meetei Mayek Vowel Sign ..Meetei Mayek Vowel Sign
        intArrayOf(0x0aaf6, 0x0aaf6),  // Meetei Mayek Virama     ..Meetei Mayek Virama
        intArrayOf(0x0abe5, 0x0abe5),  // Meetei Mayek Vowel Sign ..Meetei Mayek Vowel Sign
        intArrayOf(0x0abe8, 0x0abe8),  // Meetei Mayek Vowel Sign ..Meetei Mayek Vowel Sign
        intArrayOf(0x0abed, 0x0abed),  // Meetei Mayek Apun Iyek  ..Meetei Mayek Apun Iyek
        intArrayOf(0x0fb1e, 0x0fb1e),  // Hebrew Point Judeo-spani..Hebrew Point Judeo-spani
        intArrayOf(0x0fe00, 0x0fe0f),  // Variation Selector-1    ..Variation Selector-16
        intArrayOf(0x0fe20, 0x0fe2f),  // Combining Ligature Left ..Combining Cyrillic Titlo
        intArrayOf(0x101fd, 0x101fd),  // Phaistos Disc Sign Combi..Phaistos Disc Sign Combi
        intArrayOf(0x102e0, 0x102e0),  // Coptic Epact Thousands M..Coptic Epact Thousands M
        intArrayOf(0x10376, 0x1037a),  // Combining Old Permic Let..Combining Old Permic Let
        intArrayOf(0x10a01, 0x10a03),  // Kharoshthi Vowel Sign I ..Kharoshthi Vowel Sign Vo
        intArrayOf(0x10a05, 0x10a06),  // Kharoshthi Vowel Sign E ..Kharoshthi Vowel Sign O
        intArrayOf(0x10a0c, 0x10a0f),  // Kharoshthi Vowel Length ..Kharoshthi Sign Visarga
        intArrayOf(0x10a38, 0x10a3a),  // Kharoshthi Sign Bar Abov..Kharoshthi Sign Dot Belo
        intArrayOf(0x10a3f, 0x10a3f),  // Kharoshthi Virama       ..Kharoshthi Virama
        intArrayOf(0x10ae5, 0x10ae6),  // Manichaean Abbreviation ..Manichaean Abbreviation
        intArrayOf(0x10d24, 0x10d27),  // Hanifi Rohingya Sign Har..Hanifi Rohingya Sign Tas
        intArrayOf(0x10eab, 0x10eac),  // Yezidi Combining Hamza M..Yezidi Combining Madda M
        intArrayOf(0x10efd, 0x10eff),  // (nil)                   ..(nil)
        intArrayOf(0x10f46, 0x10f50),  // Sogdian Combining Dot Be..Sogdian Combining Stroke
        intArrayOf(0x10f82, 0x10f85),  // Old Uyghur Combining Dot..Old Uyghur Combining Two
        intArrayOf(0x11001, 0x11001),  // Brahmi Sign Anusvara    ..Brahmi Sign Anusvara
        intArrayOf(0x11038, 0x11046),  // Brahmi Vowel Sign Aa    ..Brahmi Virama
        intArrayOf(0x11070, 0x11070),  // Brahmi Sign Old Tamil Vi..Brahmi Sign Old Tamil Vi
        intArrayOf(0x11073, 0x11074),  // Brahmi Vowel Sign Old Ta..Brahmi Vowel Sign Old Ta
        intArrayOf(0x1107f, 0x11081),  // Brahmi Number Joiner    ..Kaithi Sign Anusvara
        intArrayOf(0x110b3, 0x110b6),  // Kaithi Vowel Sign U     ..Kaithi Vowel Sign Ai
        intArrayOf(0x110b9, 0x110ba),  // Kaithi Sign Virama      ..Kaithi Sign Nukta
        intArrayOf(0x110c2, 0x110c2),  // Kaithi Vowel Sign Vocali..Kaithi Vowel Sign Vocali
        intArrayOf(0x11100, 0x11102),  // Chakma Sign Candrabindu ..Chakma Sign Visarga
        intArrayOf(0x11127, 0x1112b),  // Chakma Vowel Sign A     ..Chakma Vowel Sign Uu
        intArrayOf(0x1112d, 0x11134),  // Chakma Vowel Sign Ai    ..Chakma Maayyaa
        intArrayOf(0x11173, 0x11173),  // Mahajani Sign Nukta     ..Mahajani Sign Nukta
        intArrayOf(0x11180, 0x11181),  // Sharada Sign Candrabindu..Sharada Sign Anusvara
        intArrayOf(0x111b6, 0x111be),  // Sharada Vowel Sign U    ..Sharada Vowel Sign O
        intArrayOf(0x111c9, 0x111cc),  // Sharada Sandhi Mark     ..Sharada Extra Short Vowe
        intArrayOf(0x111cf, 0x111cf),  // Sharada Sign Inverted Ca..Sharada Sign Inverted Ca
        intArrayOf(0x1122f, 0x11231),  // Khojki Vowel Sign U     ..Khojki Vowel Sign Ai
        intArrayOf(0x11234, 0x11234),  // Khojki Sign Anusvara    ..Khojki Sign Anusvara
        intArrayOf(0x11236, 0x11237),  // Khojki Sign Nukta       ..Khojki Sign Shadda
        intArrayOf(0x1123e, 0x1123e),  // Khojki Sign Sukun       ..Khojki Sign Sukun
        intArrayOf(0x11241, 0x11241),  // (nil)                   ..(nil)
        intArrayOf(0x112df, 0x112df),  // Khudawadi Sign Anusvara ..Khudawadi Sign Anusvara
        intArrayOf(0x112e3, 0x112ea),  // Khudawadi Vowel Sign U  ..Khudawadi Sign Virama
        intArrayOf(0x11300, 0x11301),  // Grantha Sign Combining A..Grantha Sign Candrabindu
        intArrayOf(0x1133b, 0x1133c),  // Combining Bindu Below   ..Grantha Sign Nukta
        intArrayOf(0x11340, 0x11340),  // Grantha Vowel Sign Ii   ..Grantha Vowel Sign Ii
        intArrayOf(0x11366, 0x1136c),  // Combining Grantha Digit ..Combining Grantha Digit
        intArrayOf(0x11370, 0x11374),  // Combining Grantha Letter..Combining Grantha Letter
        intArrayOf(0x11438, 0x1143f),  // Newa Vowel Sign U       ..Newa Vowel Sign Ai
        intArrayOf(0x11442, 0x11444),  // Newa Sign Virama        ..Newa Sign Anusvara
        intArrayOf(0x11446, 0x11446),  // Newa Sign Nukta         ..Newa Sign Nukta
        intArrayOf(0x1145e, 0x1145e),  // Newa Sandhi Mark        ..Newa Sandhi Mark
        intArrayOf(0x114b3, 0x114b8),  // Tirhuta Vowel Sign U    ..Tirhuta Vowel Sign Vocal
        intArrayOf(0x114ba, 0x114ba),  // Tirhuta Vowel Sign Short..Tirhuta Vowel Sign Short
        intArrayOf(0x114bf, 0x114c0),  // Tirhuta Sign Candrabindu..Tirhuta Sign Anusvara
        intArrayOf(0x114c2, 0x114c3),  // Tirhuta Sign Virama     ..Tirhuta Sign Nukta
        intArrayOf(0x115b2, 0x115b5),  // Siddham Vowel Sign U    ..Siddham Vowel Sign Vocal
        intArrayOf(0x115bc, 0x115bd),  // Siddham Sign Candrabindu..Siddham Sign Anusvara
        intArrayOf(0x115bf, 0x115c0),  // Siddham Sign Virama     ..Siddham Sign Nukta
        intArrayOf(0x115dc, 0x115dd),  // Siddham Vowel Sign Alter..Siddham Vowel Sign Alter
        intArrayOf(0x11633, 0x1163a),  // Modi Vowel Sign U       ..Modi Vowel Sign Ai
        intArrayOf(0x1163d, 0x1163d),  // Modi Sign Anusvara      ..Modi Sign Anusvara
        intArrayOf(0x1163f, 0x11640),  // Modi Sign Virama        ..Modi Sign Ardhacandra
        intArrayOf(0x116ab, 0x116ab),  // Takri Sign Anusvara     ..Takri Sign Anusvara
        intArrayOf(0x116ad, 0x116ad),  // Takri Vowel Sign Aa     ..Takri Vowel Sign Aa
        intArrayOf(0x116b0, 0x116b5),  // Takri Vowel Sign U      ..Takri Vowel Sign Au
        intArrayOf(0x116b7, 0x116b7),  // Takri Sign Nukta        ..Takri Sign Nukta
        intArrayOf(0x1171d, 0x1171f),  // Ahom Consonant Sign Medi..Ahom Consonant Sign Medi
        intArrayOf(0x11722, 0x11725),  // Ahom Vowel Sign I       ..Ahom Vowel Sign Uu
        intArrayOf(0x11727, 0x1172b),  // Ahom Vowel Sign Aw      ..Ahom Sign Killer
        intArrayOf(0x1182f, 0x11837),  // Dogra Vowel Sign U      ..Dogra Sign Anusvara
        intArrayOf(0x11839, 0x1183a),  // Dogra Sign Virama       ..Dogra Sign Nukta
        intArrayOf(0x1193b, 0x1193c),  // Dives Akuru Sign Anusvar..Dives Akuru Sign Candrab
        intArrayOf(0x1193e, 0x1193e),  // Dives Akuru Virama      ..Dives Akuru Virama
        intArrayOf(0x11943, 0x11943),  // Dives Akuru Sign Nukta  ..Dives Akuru Sign Nukta
        intArrayOf(0x119d4, 0x119d7),  // Nandinagari Vowel Sign U..Nandinagari Vowel Sign V
        intArrayOf(0x119da, 0x119db),  // Nandinagari Vowel Sign E..Nandinagari Vowel Sign A
        intArrayOf(0x119e0, 0x119e0),  // Nandinagari Sign Virama ..Nandinagari Sign Virama
        intArrayOf(0x11a01, 0x11a0a),  // Zanabazar Square Vowel S..Zanabazar Square Vowel L
        intArrayOf(0x11a33, 0x11a38),  // Zanabazar Square Final C..Zanabazar Square Sign An
        intArrayOf(0x11a3b, 0x11a3e),  // Zanabazar Square Cluster..Zanabazar Square Cluster
        intArrayOf(0x11a47, 0x11a47),  // Zanabazar Square Subjoin..Zanabazar Square Subjoin
        intArrayOf(0x11a51, 0x11a56),  // Soyombo Vowel Sign I    ..Soyombo Vowel Sign Oe
        intArrayOf(0x11a59, 0x11a5b),  // Soyombo Vowel Sign Vocal..Soyombo Vowel Length Mar
        intArrayOf(0x11a8a, 0x11a96),  // Soyombo Final Consonant ..Soyombo Sign Anusvara
        intArrayOf(0x11a98, 0x11a99),  // Soyombo Gemination Mark ..Soyombo Subjoiner
        intArrayOf(0x11c30, 0x11c36),  // Bhaiksuki Vowel Sign I  ..Bhaiksuki Vowel Sign Voc
        intArrayOf(0x11c38, 0x11c3d),  // Bhaiksuki Vowel Sign E  ..Bhaiksuki Sign Anusvara
        intArrayOf(0x11c3f, 0x11c3f),  // Bhaiksuki Sign Virama   ..Bhaiksuki Sign Virama
        intArrayOf(0x11c92, 0x11ca7),  // Marchen Subjoined Letter..Marchen Subjoined Letter
        intArrayOf(0x11caa, 0x11cb0),  // Marchen Subjoined Letter..Marchen Vowel Sign Aa
        intArrayOf(0x11cb2, 0x11cb3),  // Marchen Vowel Sign U    ..Marchen Vowel Sign E
        intArrayOf(0x11cb5, 0x11cb6),  // Marchen Sign Anusvara   ..Marchen Sign Candrabindu
        intArrayOf(0x11d31, 0x11d36),  // Masaram Gondi Vowel Sign..Masaram Gondi Vowel Sign
        intArrayOf(0x11d3a, 0x11d3a),  // Masaram Gondi Vowel Sign..Masaram Gondi Vowel Sign
        intArrayOf(0x11d3c, 0x11d3d),  // Masaram Gondi Vowel Sign..Masaram Gondi Vowel Sign
        intArrayOf(0x11d3f, 0x11d45),  // Masaram Gondi Vowel Sign..Masaram Gondi Virama
        intArrayOf(0x11d47, 0x11d47),  // Masaram Gondi Ra-kara   ..Masaram Gondi Ra-kara
        intArrayOf(0x11d90, 0x11d91),  // Gunjala Gondi Vowel Sign..Gunjala Gondi Vowel Sign
        intArrayOf(0x11d95, 0x11d95),  // Gunjala Gondi Sign Anusv..Gunjala Gondi Sign Anusv
        intArrayOf(0x11d97, 0x11d97),  // Gunjala Gondi Virama    ..Gunjala Gondi Virama
        intArrayOf(0x11ef3, 0x11ef4),  // Makasar Vowel Sign I    ..Makasar Vowel Sign U
        intArrayOf(0x11f00, 0x11f01),  // (nil)                   ..(nil)
        intArrayOf(0x11f36, 0x11f3a),  // (nil)                   ..(nil)
        intArrayOf(0x11f40, 0x11f40),  // (nil)                   ..(nil)
        intArrayOf(0x11f42, 0x11f42),  // (nil)                   ..(nil)
        intArrayOf(0x13440, 0x13440),  // (nil)                   ..(nil)
        intArrayOf(0x13447, 0x13455),  // (nil)                   ..(nil)
        intArrayOf(0x16af0, 0x16af4),  // Bassa Vah Combining High..Bassa Vah Combining High
        intArrayOf(0x16b30, 0x16b36),  // Pahawh Hmong Mark Cim Tu..Pahawh Hmong Mark Cim Ta
        intArrayOf(0x16f4f, 0x16f4f),  // Miao Sign Consonant Modi..Miao Sign Consonant Modi
        intArrayOf(0x16f8f, 0x16f92),  // Miao Tone Right         ..Miao Tone Below
        intArrayOf(0x16fe4, 0x16fe4),  // Khitan Small Script Fill..Khitan Small Script Fill
        intArrayOf(0x1bc9d, 0x1bc9e),  // Duployan Thick Letter Se..Duployan Double Mark
        intArrayOf(0x1cf00, 0x1cf2d),  // Znamenny Combining Mark ..Znamenny Combining Mark
        intArrayOf(0x1cf30, 0x1cf46),  // Znamenny Combining Tonal..Znamenny Priznak Modifie
        intArrayOf(0x1d167, 0x1d169),  // Musical Symbol Combining..Musical Symbol Combining
        intArrayOf(0x1d17b, 0x1d182),  // Musical Symbol Combining..Musical Symbol Combining
        intArrayOf(0x1d185, 0x1d18b),  // Musical Symbol Combining..Musical Symbol Combining
        intArrayOf(0x1d1aa, 0x1d1ad),  // Musical Symbol Combining..Musical Symbol Combining
        intArrayOf(0x1d242, 0x1d244),  // Combining Greek Musical ..Combining Greek Musical
        intArrayOf(0x1da00, 0x1da36),  // Signwriting Head Rim    ..Signwriting Air Sucking
        intArrayOf(0x1da3b, 0x1da6c),  // Signwriting Mouth Closed..Signwriting Excitement
        intArrayOf(0x1da75, 0x1da75),  // Signwriting Upper Body T..Signwriting Upper Body T
        intArrayOf(0x1da84, 0x1da84),  // Signwriting Location Hea..Signwriting Location Hea
        intArrayOf(0x1da9b, 0x1da9f),  // Signwriting Fill Modifie..Signwriting Fill Modifie
        intArrayOf(0x1daa1, 0x1daaf),  // Signwriting Rotation Mod..Signwriting Rotation Mod
        intArrayOf(0x1e000, 0x1e006),  // Combining Glagolitic Let..Combining Glagolitic Let
        intArrayOf(0x1e008, 0x1e018),  // Combining Glagolitic Let..Combining Glagolitic Let
        intArrayOf(0x1e01b, 0x1e021),  // Combining Glagolitic Let..Combining Glagolitic Let
        intArrayOf(0x1e023, 0x1e024),  // Combining Glagolitic Let..Combining Glagolitic Let
        intArrayOf(0x1e026, 0x1e02a),  // Combining Glagolitic Let..Combining Glagolitic Let
        intArrayOf(0x1e08f, 0x1e08f),  // (nil)                   ..(nil)
        intArrayOf(0x1e130, 0x1e136),  // Nyiakeng Puachue Hmong T..Nyiakeng Puachue Hmong T
        intArrayOf(0x1e2ae, 0x1e2ae),  // Toto Sign Rising Tone   ..Toto Sign Rising Tone
        intArrayOf(0x1e2ec, 0x1e2ef),  // Wancho Tone Tup         ..Wancho Tone Koini
        intArrayOf(0x1e4ec, 0x1e4ef),  // (nil)                   ..(nil)
        intArrayOf(0x1e8d0, 0x1e8d6),  // Mende Kikakui Combining ..Mende Kikakui Combining
        intArrayOf(0x1e944, 0x1e94a),  // Adlam Alif Lengthener   ..Adlam Nukta
        intArrayOf(0xe0100, 0xe01ef),  // Variation Selector-17   ..Variation Selector-256
    )

    // https://github.com/jquast/wcwidth/blob/master/wcwidth/table_wide.py
    // from https://github.com/jquast/wcwidth/pull/64
    // at commit 1b9b6585b0080ea5cb88dc9815796505724793fe (2022-12-16):
    private val WIDE_EASTASIAN = arrayOf(
        intArrayOf(0x01100, 0x0115f),  // Hangul Choseong Kiyeok  ..Hangul Choseong Filler
        intArrayOf(0x0231a, 0x0231b),  // Watch                   ..Hourglass
        intArrayOf(0x02329, 0x0232a),  // Left-pointing Angle Brac..Right-pointing Angle Bra
        intArrayOf(0x023e9, 0x023ec),  // Black Right-pointing Dou..Black Down-pointing Doub
        intArrayOf(0x023f0, 0x023f0),  // Alarm Clock             ..Alarm Clock
        intArrayOf(0x023f3, 0x023f3),  // Hourglass With Flowing S..Hourglass With Flowing S
        intArrayOf(0x025fd, 0x025fe),  // White Medium Small Squar..Black Medium Small Squar
        intArrayOf(0x02614, 0x02615),  // Umbrella With Rain Drops..Hot Beverage
        intArrayOf(0x02648, 0x02653),  // Aries                   ..Pisces
        intArrayOf(0x0267f, 0x0267f),  // Wheelchair Symbol       ..Wheelchair Symbol
        intArrayOf(0x02693, 0x02693),  // Anchor                  ..Anchor
        intArrayOf(0x026a1, 0x026a1),  // High Voltage Sign       ..High Voltage Sign
        intArrayOf(0x026aa, 0x026ab),  // Medium White Circle     ..Medium Black Circle
        intArrayOf(0x026bd, 0x026be),  // Soccer Ball             ..Baseball
        intArrayOf(0x026c4, 0x026c5),  // Snowman Without Snow    ..Sun Behind Cloud
        intArrayOf(0x026ce, 0x026ce),  // Ophiuchus               ..Ophiuchus
        intArrayOf(0x026d4, 0x026d4),  // No Entry                ..No Entry
        intArrayOf(0x026ea, 0x026ea),  // Church                  ..Church
        intArrayOf(0x026f2, 0x026f3),  // Fountain                ..Flag In Hole
        intArrayOf(0x026f5, 0x026f5),  // Sailboat                ..Sailboat
        intArrayOf(0x026fa, 0x026fa),  // Tent                    ..Tent
        intArrayOf(0x026fd, 0x026fd),  // Fuel Pump               ..Fuel Pump
        intArrayOf(0x02705, 0x02705),  // White Heavy Check Mark  ..White Heavy Check Mark
        intArrayOf(0x0270a, 0x0270b),  // Raised Fist             ..Raised Hand
        intArrayOf(0x02728, 0x02728),  // Sparkles                ..Sparkles
        intArrayOf(0x0274c, 0x0274c),  // Cross Mark              ..Cross Mark
        intArrayOf(0x0274e, 0x0274e),  // Negative Squared Cross M..Negative Squared Cross M
        intArrayOf(0x02753, 0x02755),  // Black Question Mark Orna..White Exclamation Mark O
        intArrayOf(0x02757, 0x02757),  // Heavy Exclamation Mark S..Heavy Exclamation Mark S
        intArrayOf(0x02795, 0x02797),  // Heavy Plus Sign         ..Heavy Division Sign
        intArrayOf(0x027b0, 0x027b0),  // Curly Loop              ..Curly Loop
        intArrayOf(0x027bf, 0x027bf),  // Double Curly Loop       ..Double Curly Loop
        intArrayOf(0x02b1b, 0x02b1c),  // Black Large Square      ..White Large Square
        intArrayOf(0x02b50, 0x02b50),  // White Medium Star       ..White Medium Star
        intArrayOf(0x02b55, 0x02b55),  // Heavy Large Circle      ..Heavy Large Circle
        intArrayOf(0x02e80, 0x02e99),  // Cjk Radical Repeat      ..Cjk Radical Rap
        intArrayOf(0x02e9b, 0x02ef3),  // Cjk Radical Choke       ..Cjk Radical C-simplified
        intArrayOf(0x02f00, 0x02fd5),  // Kangxi Radical One      ..Kangxi Radical Flute
        intArrayOf(0x02ff0, 0x02ffb),  // Ideographic Description ..Ideographic Description
        intArrayOf(0x03000, 0x0303e),  // Ideographic Space       ..Ideographic Variation In
        intArrayOf(0x03041, 0x03096),  // Hiragana Letter Small A ..Hiragana Letter Small Ke
        intArrayOf(0x03099, 0x030ff),  // Combining Katakana-hirag..Katakana Digraph Koto
        intArrayOf(0x03105, 0x0312f),  // Bopomofo Letter B       ..Bopomofo Letter Nn
        intArrayOf(0x03131, 0x0318e),  // Hangul Letter Kiyeok    ..Hangul Letter Araeae
        intArrayOf(0x03190, 0x031e3),  // Ideographic Annotation L..Cjk Stroke Q
        intArrayOf(0x031f0, 0x0321e),  // Katakana Letter Small Ku..Parenthesized Korean Cha
        intArrayOf(0x03220, 0x03247),  // Parenthesized Ideograph ..Circled Ideograph Koto
        intArrayOf(0x03250, 0x04dbf),  // Partnership Sign        ..Cjk Unified Ideograph-4d
        intArrayOf(0x04e00, 0x0a48c),  // Cjk Unified Ideograph-4e..Yi Syllable Yyr
        intArrayOf(0x0a490, 0x0a4c6),  // Yi Radical Qot          ..Yi Radical Ke
        intArrayOf(0x0a960, 0x0a97c),  // Hangul Choseong Tikeut-m..Hangul Choseong Ssangyeo
        intArrayOf(0x0ac00, 0x0d7a3),  // Hangul Syllable Ga      ..Hangul Syllable Hih
        intArrayOf(0x0f900, 0x0faff),  // Cjk Compatibility Ideogr..(nil)
        intArrayOf(0x0fe10, 0x0fe19),  // Presentation Form For Ve..Presentation Form For Ve
        intArrayOf(0x0fe30, 0x0fe52),  // Presentation Form For Ve..Small Full Stop
        intArrayOf(0x0fe54, 0x0fe66),  // Small Semicolon         ..Small Equals Sign
        intArrayOf(0x0fe68, 0x0fe6b),  // Small Reverse Solidus   ..Small Commercial At
        intArrayOf(0x0ff01, 0x0ff60),  // Fullwidth Exclamation Ma..Fullwidth Right White Pa
        intArrayOf(0x0ffe0, 0x0ffe6),  // Fullwidth Cent Sign     ..Fullwidth Won Sign
        intArrayOf(0x16fe0, 0x16fe4),  // Tangut Iteration Mark   ..Khitan Small Script Fill
        intArrayOf(0x16ff0, 0x16ff1),  // Vietnamese Alternate Rea..Vietnamese Alternate Rea
        intArrayOf(0x17000, 0x187f7),  // (nil)                   ..(nil)
        intArrayOf(0x18800, 0x18cd5),  // Tangut Component-001    ..Khitan Small Script Char
        intArrayOf(0x18d00, 0x18d08),  // (nil)                   ..(nil)
        intArrayOf(0x1aff0, 0x1aff3),  // Katakana Letter Minnan T..Katakana Letter Minnan T
        intArrayOf(0x1aff5, 0x1affb),  // Katakana Letter Minnan T..Katakana Letter Minnan N
        intArrayOf(0x1affd, 0x1affe),  // Katakana Letter Minnan N..Katakana Letter Minnan N
        intArrayOf(0x1b000, 0x1b122),  // Katakana Letter Archaic ..Katakana Letter Archaic
        intArrayOf(0x1b132, 0x1b132),  // (nil)                   ..(nil)
        intArrayOf(0x1b150, 0x1b152),  // Hiragana Letter Small Wi..Hiragana Letter Small Wo
        intArrayOf(0x1b155, 0x1b155),  // (nil)                   ..(nil)
        intArrayOf(0x1b164, 0x1b167),  // Katakana Letter Small Wi..Katakana Letter Small N
        intArrayOf(0x1b170, 0x1b2fb),  // Nushu Character-1b170   ..Nushu Character-1b2fb
        intArrayOf(0x1f004, 0x1f004),  // Mahjong Tile Red Dragon ..Mahjong Tile Red Dragon
        intArrayOf(0x1f0cf, 0x1f0cf),  // Playing Card Black Joker..Playing Card Black Joker
        intArrayOf(0x1f18e, 0x1f18e),  // Negative Squared Ab     ..Negative Squared Ab
        intArrayOf(0x1f191, 0x1f19a),  // Squared Cl              ..Squared Vs
        intArrayOf(0x1f200, 0x1f202),  // Square Hiragana Hoka    ..Squared Katakana Sa
        intArrayOf(0x1f210, 0x1f23b),  // Squared Cjk Unified Ideo..Squared Cjk Unified Ideo
        intArrayOf(0x1f240, 0x1f248),  // Tortoise Shell Bracketed..Tortoise Shell Bracketed
        intArrayOf(0x1f250, 0x1f251),  // Circled Ideograph Advant..Circled Ideograph Accept
        intArrayOf(0x1f260, 0x1f265),  // Rounded Symbol For Fu   ..Rounded Symbol For Cai
        intArrayOf(0x1f300, 0x1f320),  // Cyclone                 ..Shooting Star
        intArrayOf(0x1f32d, 0x1f335),  // Hot Dog                 ..Cactus
        intArrayOf(0x1f337, 0x1f37c),  // Tulip                   ..Baby Bottle
        intArrayOf(0x1f37e, 0x1f393),  // Bottle With Popping Cork..Graduation Cap
        intArrayOf(0x1f3a0, 0x1f3ca),  // Carousel Horse          ..Swimmer
        intArrayOf(0x1f3cf, 0x1f3d3),  // Cricket Bat And Ball    ..Table Tennis Paddle And
        intArrayOf(0x1f3e0, 0x1f3f0),  // House Building          ..European Castle
        intArrayOf(0x1f3f4, 0x1f3f4),  // Waving Black Flag       ..Waving Black Flag
        intArrayOf(0x1f3f8, 0x1f43e),  // Badminton Racquet And Sh..Paw Prints
        intArrayOf(0x1f440, 0x1f440),  // Eyes                    ..Eyes
        intArrayOf(0x1f442, 0x1f4fc),  // Ear                     ..Videocassette
        intArrayOf(0x1f4ff, 0x1f53d),  // Prayer Beads            ..Down-pointing Small Red
        intArrayOf(0x1f54b, 0x1f54e),  // Kaaba                   ..Menorah With Nine Branch
        intArrayOf(0x1f550, 0x1f567),  // Clock Face One Oclock   ..Clock Face Twelve-thirty
        intArrayOf(0x1f57a, 0x1f57a),  // Man Dancing             ..Man Dancing
        intArrayOf(0x1f595, 0x1f596),  // Reversed Hand With Middl..Raised Hand With Part Be
        intArrayOf(0x1f5a4, 0x1f5a4),  // Black Heart             ..Black Heart
        intArrayOf(0x1f5fb, 0x1f64f),  // Mount Fuji              ..Person With Folded Hands
        intArrayOf(0x1f680, 0x1f6c5),  // Rocket                  ..Left Luggage
        intArrayOf(0x1f6cc, 0x1f6cc),  // Sleeping Accommodation  ..Sleeping Accommodation
        intArrayOf(0x1f6d0, 0x1f6d2),  // Place Of Worship        ..Shopping Trolley
        intArrayOf(0x1f6d5, 0x1f6d7),  // Hindu Temple            ..Elevator
        intArrayOf(0x1f6dc, 0x1f6df),  // (nil)                   ..Ring Buoy
        intArrayOf(0x1f6eb, 0x1f6ec),  // Airplane Departure      ..Airplane Arriving
        intArrayOf(0x1f6f4, 0x1f6fc),  // Scooter                 ..Roller Skate
        intArrayOf(0x1f7e0, 0x1f7eb),  // Large Orange Circle     ..Large Brown Square
        intArrayOf(0x1f7f0, 0x1f7f0),  // Heavy Equals Sign       ..Heavy Equals Sign
        intArrayOf(0x1f90c, 0x1f93a),  // Pinched Fingers         ..Fencer
        intArrayOf(0x1f93c, 0x1f945),  // Wrestlers               ..Goal Net
        intArrayOf(0x1f947, 0x1f9ff),  // First Place Medal       ..Nazar Amulet
        intArrayOf(0x1fa70, 0x1fa7c),  // Ballet Shoes            ..Crutch
        intArrayOf(0x1fa80, 0x1fa88),  // Yo-yo                   ..(nil)
        intArrayOf(0x1fa90, 0x1fabd),  // Ringed Planet           ..(nil)
        intArrayOf(0x1fabf, 0x1fac5),  // (nil)                   ..Person With Crown
        intArrayOf(0x1face, 0x1fadb),  // (nil)                   ..(nil)
        intArrayOf(0x1fae0, 0x1fae8),  // Melting Face            ..(nil)
        intArrayOf(0x1faf0, 0x1faf8),  // Hand With Index Finger A..(nil)
        intArrayOf(0x20000, 0x2fffd),  // Cjk Unified Ideograph-20..(nil)
        intArrayOf(0x30000, 0x3fffd),  // Cjk Unified Ideograph-30..(nil)
    )

    private fun intable(table: Array<IntArray>, c: Int): Boolean {
        if (c < table[0][0]) return false
        var bot = 0
        var top = table.size - 1
        while (top >= bot) {
            val mid = (bot + top) / 2
            if (table[mid][1] < c) {
                bot = mid + 1
            } else if (table[mid][0] > c) {
                top = mid - 1
            } else {
                return true
            }
        }
        return false
    }

    /** Return the terminal display width of a code point: 0, 1 or 2. */
    fun width(ucs: Int): Int {
        if (ucs == 0 ||
            ucs == 0x034F ||
            (ucs in 0x200B..0x200F) ||
            ucs == 0x2028 ||
            ucs == 0x2029 ||
            (ucs in 0x202A..0x202E) ||
            (ucs in 0x2060..0x2063)) {
            return 0
        }

        // C0/C1 control characters
        // Termux change: Return 0 instead of -1.
        if (ucs < 32 || (ucs in 0x07F until 0x0A0)) return 0

        if (intable(ZERO_WIDTH, ucs)) return 0

        return if (intable(WIDE_EASTASIAN, ucs)) 2 else 1
    }

    /** The width at an index position in a java char array. */
    fun width(chars: CharArray, index: Int): Int {
        val c = chars[index]
        return if (Character.isHighSurrogate(c)) width(Character.toCodePoint(c, chars[index + 1])) else width(c.code)
    }

    /**
     * The zero width characters count like combining characters in the `chars` array from start
     * index to end index (exclusive).
     */
    fun zeroWidthCharsCount(chars: CharArray, start: Int, end: Int): Int {
        if (start < 0 || start >= chars.size) return 0
        var count = 0
        var i = start
        while (i < end && i < chars.size) {
            if (Character.isHighSurrogate(chars[i])) {
                if (width(Character.toCodePoint(chars[i], chars[i + 1])) <= 0) {
                    count++
                }
                i += 2
            } else {
                if (width(chars[i].code) <= 0) {
                    count++
                }
                i++
            }
        }
        return count
    }
}
