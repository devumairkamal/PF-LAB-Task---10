#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "books.dat"
#define TEMP_NAME "books_temp.dat"
#define TEXT_SIZE 100
#define BUF_SIZE  256
typedef struct {
    int  id;
    char title[TEXT_SIZE];
    char author[TEXT_SIZE];
    int  copies;
} Book;
static int read_line(char *buffer, int size)
{
    size_t len;
    int ch;
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return 0;
    }
    len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }
    return 1;
}
static int read_int(const char *prompt, int min, int *value)
{
    char buffer[BUF_SIZE];
    char *end;
    long v;
    for (;;) {
        printf("%s", prompt);
        if (!read_line(buffer, BUF_SIZE))
            return 0;
        v = strtol(buffer, &end, 10);
        if (end == buffer) {
            printf("Please enter a whole number.\n");
            continue;
        }
        while (isspace((unsigned char)*end))
            end++;
        if (*end != '\0')
            printf("Please enter a whole number.\n");
        else if (v < min)
            printf("The value must be %d or more.\n", min);
        else {
            *value = (int)v;
            return 1;
        }
    }
}
static int read_text(const char *prompt, char *field)
{
    for (;;) {
        printf("%s", prompt);
        if (!read_line(field, TEXT_SIZE))
            return 0;
        if (strlen(field) > 0)
            return 1;
        printf("This field cannot be empty.\n");
    }
}
static int book_exists(int id)
{
    FILE *fp = fopen(FILE_NAME, "rb");
    Book b;
    int found = 0;
    if (fp == NULL)
        return 0;
    while (!found && fread(&b, sizeof b, 1, fp) == 1) {
        if (b.id == id)
            found = 1;
    }
    fclose(fp);
    return found;
}
static void print_table_header(void)
{
    printf("%-8s %-30s %-25s %8s\n", "ID", "Title", "Author", "Copies");
    printf("-----------       ------------          --------    --------- \n");
}
static void print_table_row(const Book *b)
{
    printf("%-8d %-30s %-25s %8d\n", b->id, b->title, b->author, b->copies);
}
static void print_details(const Book *b)
{
    printf("\nBook ID          : %d\n", b->id);
    printf("Title            : %s\n", b->title);
    printf("Author           : %s\n", b->author);
    printf("Copies available : %d\n", b->copies);
}
static void add_book(void)
{
    Book b;
    FILE *fp;
    printf("\n--- Add New Book ---\n");
    if (!read_int("Book ID: ", 1, &b.id))
        return;
    if (book_exists(b.id)) {
        printf("A book with ID %d already exists. The book was not added.\n", b.id);
        return;
    }
    if (!read_text("Title: ", b.title))
        return;
    if (!read_text("Author name: ", b.author))
        return;
    if (!read_int("Available copies: ", 0, &b.copies))
        return;
    fp = fopen(FILE_NAME, "ab");
    if (fp == NULL) {
        perror("Error: could not open books.dat");
        return;
    }
    if (fwrite(&b, sizeof b, 1, fp) == 1)
        printf("Book added successfully.\n");
    else
        printf("Error: the record could not be written.\n");
    fclose(fp);
}
static void display_all(void)
{
    FILE *fp = fopen(FILE_NAME, "rb");
    Book b;
    int count = 0;
    if (fp == NULL) {
        if (errno == ENOENT)
            printf("\nNo books are stored yet.\n");
        else
            perror("Error: could not open books.dat");
        return;
    }
    printf("\n--- All Books ---\n");
    print_table_header();
    while (fread(&b, sizeof b, 1, fp) == 1) {
        print_table_row(&b);
        count++;
    }
    fclose(fp);
    if (count == 0)
        printf("No books are stored yet.\n");
    else
        printf("\nTotal books: %d\n", count);
}
static void search_by_id(void)
{
    int id;
    FILE *fp;
    Book b;
    int found = 0;
    if (!read_int("Enter book ID to search: ", 1, &id))
        return;
    fp = fopen(FILE_NAME, "rb");
    if (fp == NULL) {
        if (errno == ENOENT)
            printf("No book found with ID %d.\n", id);
        else
            perror("Error: could not open books.dat");
        return;
    }
    while (!found && fread(&b, sizeof b, 1, fp) == 1) {
        if (b.id == id) {
            found = 1;
            print_details(&b);
        }
    }
    fclose(fp);
    if (!found)
        printf("No book found with ID %d.\n", id);
}
static void update_copies(void)
{
    int id;
    int new_copies;
    long pos = 0;
    FILE *fp;
    Book b;
    int found = 0;
    if (!read_int("Enter book ID to update: ", 1, &id))
        return;
    fp = fopen(FILE_NAME, "r+b");
    if (fp == NULL) {
        if (errno == ENOENT)
            printf("No book found with ID %d.\n", id);
        else
            perror("Error: could not open books.dat");
        return;
    }
    while (!found) {
        pos = ftell(fp);
        if (fread(&b, sizeof b, 1, fp) != 1)
            break;
        if (b.id == id)
            found = 1;
    }
    if (!found) {
        fclose(fp);
        printf("No book found with ID %d. Nothing was updated.\n", id);
        return;
    }
    printf("Updating \"%s\" (currently %d copies).\n", b.title, b.copies);
    if (!read_int("New number of available copies: ", 0, &new_copies)) {
        fclose(fp);
        return;
    }
    b.copies = new_copies;
    fseek(fp, pos, SEEK_SET);
    if (fwrite(&b, sizeof b, 1, fp) == 1)
        printf("Copies updated successfully.\n");
    else
        printf("Error: the record could not be updated.\n");
    fclose(fp);
}
static void delete_book(void)
{
    int id;
    FILE *src;
    FILE *dst;
    Book b;
    int found = 0;
    int write_ok = 1;
    if (!read_int("Enter book ID to delete: ", 1, &id))
        return;
    src = fopen(FILE_NAME, "rb");
    if (src == NULL) {
        if (errno == ENOENT)
            printf("No book found with ID %d.\n", id);
        else
            perror("Error: could not open books.dat");
        return;
    }
    dst = fopen(TEMP_NAME, "wb");
    if (dst == NULL) {
        perror("Error: could not create a temporary file");
        fclose(src);
        return;
   		}
    while (write_ok && fread(&b, sizeof b, 1, src) == 1) {
        if (b.id == id)
            found = 1;
        else if (fwrite(&b, sizeof b, 1, dst) != 1)
            write_ok = 0;
    }
    fclose(src);
    fclose(dst);
    if (!write_ok) {
        remove(TEMP_NAME);
        printf("Error: the new file could not be written. books.dat was not changed.\n");
        return;
    }
    if (!found) {
        remove(TEMP_NAME);
        printf("No book found with ID %d. Nothing was deleted.\n", id);
        return;
    }
    if (remove(FILE_NAME) != 0 || rename(TEMP_NAME, FILE_NAME) != 0) {
        perror("Error: could not replace books.dat");
        return;
    }
    printf("Book with ID %d has been deleted.\n", id);
}
int main(void)
{
    int choice;
    printf("- - - -Library Book Record System- - - - -\n");
    for (;;) {
        printf("\n1. Add a new book\n");
        printf("2. Display all books\n");
        printf("3. Search book by ID\n");
        printf("4. Update available copies\n");
        printf("5. Delete a book\n");
        printf("6. Exit\n");
        if (!read_int("Choose an option: ", 1, &choice))
            break;
        switch (choice) {
        case 1:
            add_book();
            break;
        case 2:
            display_all();
            break;
        case 3:
            search_by_id();
            break;
        case 4:
            update_copies();
            break;
        case 5:
            delete_book();
            break;
        case 6:
            printf("Goodbye!\n");
            return EXIT_SUCCESS;
        default:
            printf("Invalid option. Please enter a number from 1 to 6.\n");
            break;
        }
    }
    return EXIT_SUCCESS;
}

