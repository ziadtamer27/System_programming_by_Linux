
int pwd_main() {
    char cwd[1024];  

   
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);  
    } else {
        perror("getcwd");  
    }
    return 0;
}
