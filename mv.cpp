int mv_main(int argc, char *argv[]) {
   if (argc != 3) {
        fprintf(stderr, "Usage: mv <source_file> <destination_file>\n");
        return 1;
    }

    const char *source = argv[1];
    const char *destination = argv[2];

    if (rename(source, destination) != 0) {
        perror("Error moving file");
        return 1;
    }

    return 0;
}
