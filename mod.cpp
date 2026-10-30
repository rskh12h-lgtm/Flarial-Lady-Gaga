// ByteMod - mode d'actions d'inventaire PERMANENT (style Flarial "quick move")
// Le mode est global : il ne se réinitialise jamais quand on ferme/rouvre
// l'inventaire, un coffre, un four, etc. Il est aussi sauvegardé sur disque
// pour survivre à un redémarrage du jeu.
#include <jni.h>
#include <android/log.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

#define TAG "ByteMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

enum Mode : int {
    MODE_NORMAL = 0,
    MODE_MOVE_STACK,
    MODE_MOVE_COLUMN,
    MODE_MOVE_ROW,
    MODE_DROP_ONE,
    MODE_DROP_STACK,
};

static std::atomic<int> g_mode{MODE_NORMAL};
static const char* kCfgPath = "/storage/emulated/0/games/com.mojang/bytemod_mode.cfg";

static void save_mode() {
    if (FILE* f = fopen(kCfgPath, "w")) { fprintf(f, "%d", g_mode.load()); fclose(f); }
}
static void load_mode() {
    if (FILE* f = fopen(kCfgPath, "r")) {
        int m = 0;
        if (fscanf(f, "%d", &m) == 1 && m >= 0 && m <= MODE_DROP_STACK) g_mode = m;
        fclose(f);
    }
}

// API appelable depuis le launcher (JNI/dlsym) quand tu tapes un bouton du menu
extern "C" __attribute__((visibility("default"))) void bytemod_set_mode(int m) {
    if (m < 0 || m > MODE_DROP_STACK) return;
    g_mode = m;
    save_mode();
    LOGI("mode = %d", m);
}
extern "C" __attribute__((visibility("default"))) int bytemod_get_mode() { return g_mode; }

// ---------------------------------------------------------------------------
// HOOK : un seul point d'entrée commun à TOUS les écrans à slots
// (inventaire, coffre, tonneau, four, établi, enclume, shulker...).
// Dans Bedrock c'est la logique de ContainerScreenController (clic sur un slot).
// Il faut hooker cette fonction dans libminecraftpe.so de la 26.60 :
//   - trouver la signature/offset (la lib est strippée -> pattern scan)
//   - brancher avec une lib de hook (Dobby / ShadowHook)
// ---------------------------------------------------------------------------
typedef void (*SlotTapFn)(void* controller, void* collectionName, int slot);
static SlotTapFn orig_slot_tap = nullptr;

static void on_slot_tap(void* controller, void* collectionName, int slot) {
    switch (g_mode.load()) {
        case MODE_NORMAL:
            orig_slot_tap(controller, collectionName, slot);
            return;
        case MODE_MOVE_STACK:
            // TODO: envoyer la pile du slot vers l'autre conteneur (shift-click)
            break;
        case MODE_MOVE_COLUMN:
            // TODO: répéter le transfert sur tous les slots de la même colonne
            break;
        case MODE_MOVE_ROW:
            // TODO: idem sur toute la ligne
            break;
        case MODE_DROP_ONE:
            // TODO: jeter 1 objet du slot
            break;
        case MODE_DROP_STACK:
            // TODO: jeter toute la pile
            break;
    }
}

static void install_hooks() {
    // TODO: résoudre l'adresse de la fonction de clic de slot (26.60)
    // void* target = find_pattern("libminecraftpe.so", "<SIGNATURE>");
    // hook(target, (void*)on_slot_tap, (void**)&orig_slot_tap);
    LOGI("hooks: signature 26.60 manquante, non installés");
}

static void* mod_thread(void*) {
    sleep(5); // attendre le chargement de libminecraftpe.so
    install_hooks();
    return nullptr;
}

__attribute__((constructor))
static void mod_init() {
    load_mode(); // restaure le dernier mode choisi
    LOGI("ByteMod chargé, mode restauré = %d", g_mode.load());
    pthread_t t;
    pthread_create(&t, nullptr, mod_thread, nullptr);
    pthread_detach(t);
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) { return JNI_VERSION_1_6; }
