# (WIP) Human passwords
I suck at remembering random information. Whenever i type a new password that is made up of random characters, I need to check the password every few characters. For fun and curiosity, i want to make this tool to generate passwords from english words - perhaps they will be easier/funner to type and fun to remember.

At this point, the project is just a collection of related functionality that has not been assembled into a working product. Once the enough components are assembled and use is possible, this is where the documentation will be.



Notes for development:

save file structure:
- length of the huffman tree section (the amount of nodes)
- for each leaf of the huffman tree: 1 byte for the char, 1 byte for the length of the code, then the code (using bits)
- stream of words to be read using the huffman tree (using a self-chosen seperator)
- EOF
