"""Native-phone examples for Persian سلام, used by the private probes.

eSpeak's Persian-script reference is s a l A m. Its short front `a` maps
here to Finnish `A` (ä); its back `A` maps to the independently tuned
Finnish `a`. The uppercase letters are ECI phoneme names, not spelling.
"""

SALAM = '`[.0sA.1lam]'
SALAM_PREVIOUS = '`[.0sa.1lam]'
PERSIAN_PHRASE = SALAM + ' `[.1do.0nja]'

# The private Q=6 probe treats this manually specified Finnish /k/ as the
# selected uvular experiment. Both uppercase A phones are Finnish /ä/.
QALAM = '`[.0kA.1lAm]'
