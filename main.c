#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/gl.h>

#include "philosopher.h"

#define WIDTH 800.0F
#define HEIGHT 600.0F

sem_t mutex;

Philosopher **phi;

void displayFunc() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WIDTH / 2.0f, WIDTH / 2.0f, -HEIGHT / 2.0f, HEIGHT / 2.0f, -1.0f,
            1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Desenhando os Filósofos :)
    for (int i = 0; i < PHILOSOPHERS; i++) {
        glPushMatrix();

        glTranslated(cos(i * (2 * M_PI) / PHILOSOPHERS) * 128.0f,
                     sin(i * (2 * M_PI) / PHILOSOPHERS) * 128.0f, 0.0f);

        glColor3d((double)(phi[i]->state == HUNGRY),
                  (double)(phi[i]->state == EATING),
                  (double)(phi[i]->state == THINKING));
        glBegin(GL_TRIANGLE_FAN);
        for (int j = 0; j < 24; j++)
            glVertex2d(cos(j * (2 * M_PI) / 24) * 32.0f,
                       sin(j * (2 * M_PI) / 24) * 32.0f);
        glEnd();

        glColor3d(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j <= 24; j++)
            glVertex2d(cos(j * (2 * M_PI) / 24) * 32.0f,
                       sin(j * (2 * M_PI) / 24) * 32.0f);
        glEnd();

        glPopMatrix();
    }

    glutSwapBuffers();
}

void timer() {
    glutTimerFunc(1000.0f / 60.0f, timer, 0);
    glutPostRedisplay();
}

void *callGlut() {
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Jantar dos Filósofos");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glEnable(GL_MULTISAMPLE);

    glutDisplayFunc(displayFunc);
    glutTimerFunc(1000.0f / 30.0f, timer, 0);
    glutMainLoop();

    return NULL;
}

Philosopher **allocTable() {
    Philosopher **phi =
        (Philosopher **)malloc(sizeof(Philosopher *) * PHILOSOPHERS);

    for (int i = 0; i < PHILOSOPHERS; i++) {
        phi[i] = (Philosopher *)malloc(sizeof(Philosopher));

        phi[i]->id = i;
        phi[i]->state = THINKING;
        phi[i]->l = NULL;
        phi[i]->r = NULL;

        sem_init(&(phi[i]->chopsticks), 0, 0);
    }
    for (int i = 0; i < PHILOSOPHERS; i++) {
        phi[i]->l = phi[LEFT(i)];
        phi[i]->r = phi[RIGHT(i)];
    }

    return phi;
}

void deallocTable() {
    for (int i = 0; i < PHILOSOPHERS; i++) {
        free(phi[i]);
    }
    free(phi);
}

void checkEat(Philosopher *phi) {
    if (phi->state == HUNGRY && phi->l->state != EATING &&
        phi->r->state != EATING) {
        phi->state = EATING;
        printf("Philosopher %i is EATING!\n", phi->id);
        sem_post(&(phi->chopsticks));
    }
}

void takeChopsticks(Philosopher *phi) {
    sem_wait(&mutex);

    phi->state = HUNGRY;
    printf("Philosopher %i is HUNGRY!\n", phi->id);
    checkEat(phi);

    sem_post(&mutex);
    sem_wait(&(phi->chopsticks));
}

void putChopsticks(Philosopher *phi) {
    sem_wait(&mutex);

    phi->state = THINKING;
    printf("Philosopher %i is THINKING!\n", phi->id);

    checkEat(phi->l);
    checkEat(phi->r);

    sem_post(&mutex);
}

void *callPhilosopher(void *phi) {
    while (1) {
        sleep(1);
        takeChopsticks(phi);
        sleep(2);
        putChopsticks(phi);
    }
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);

    sem_init(&mutex, 0, 1);
    pthread_t phi_thread_id[PHILOSOPHERS];
    pthread_t glut_thread_id;

    // Generating the table where the Philosophers sit in :)
    phi = allocTable();

    // The dinner starts...
    for (int i = 0; i < PHILOSOPHERS; i++) {
        pthread_create(&phi_thread_id[i], NULL, callPhilosopher, phi[i]);
    }
    pthread_create(&glut_thread_id, NULL, callGlut, NULL);
    for (int i = 0; i < PHILOSOPHERS; i++) {
        pthread_join(phi_thread_id[i], NULL);
    }
    pthread_join(glut_thread_id, NULL);

    deallocTable();
    return EXIT_SUCCESS;
}
