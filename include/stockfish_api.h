/**
 * @file stockfish_api.h
 *
 * @brief Integrates with stockfish
 *
 * @author Gustavo Bacagine <gustavo.bacagine@protonmail.com>
 *
 * @date Jul 2025
 */

#ifndef _STOCKFISH_API_H_
#define _STOCKFISH_API_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef _WIN32
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/wait.h>
#endif

#define __STOCKFISH_API_VERSION__ "0.1"

/**
 * @brief ...
 *
 */
typedef struct STRUCT_MOVEMENT {
  int iStartX;
  int iStartY;
  int iEndX;
  int iEndY;
  char szMovement[32];
} STRUCT_MOVEMENT, *PSTRUCT_MOVEMENT;

/**
 * @brief ...
 *
 */
typedef struct STRUCT_MOVE_LIST {
  struct STRUCT_MOVEMENT stMovement;
  struct STRUCT_MOVE_LIST *pstNext;
} STRUCT_MOVE_LIST, *PSTRUCT_MOVE_LIST;

extern STRUCT_MOVE_LIST *gpstMoveList;

typedef struct STRUCT_STOCKFISH {
  char szBuffer[2048];
  char szMoveTime[16];
  char szBestMove[32];
  int bStockfishOn;
} STRUCT_STOCKFISH, *PSTRUCT_STOCKFISH;

extern STRUCT_STOCKFISH gstStockfish;

#ifndef _WIN32
extern pid_t pidStockfish;
extern int fdToStockfish[2];
extern int fdFromStockfish[2];
#endif

/**
 * @brief ...
 *
 */
void vSTOCKFISH_UCI(void);

/**
 * @brief ...
 *
 * @param kpszPosition p_kpszPosition:...
 */
void vSTOCKFISH_Position(void);

/**
 * @brief ...
 *
 * @param kpszMovetime p_kpszMovetime:...
 */
void vSTOCKFISH_GoMovetime(void);

/**
 * @brief ...
 *
 * @return int
 */
int bSTOCKFISH_IsReady(void);

/**
 * @brief ...
 *
 */
void vSTOCKFISH_Quit(void);

/**
 * @brief ...
 *
 * @return TRUE
 * @return FALSE
 */
int bSTOCKFISH_Init(const char *kpszStockfishPath, const char *kpszMoveTime);

/**
 * @brief Check if comunication with stockfish was started
 *
 * @return TRUE Comunication was started
 * @return FALSE Comunication is not started
 */
int bSTOCKFISH_IsStarted(void);

/**
 * @brief Finish comunication with stockfish
 */
void vSTOCKFISH_End(void);

/**
 * @brief Get best movement
 *
 * Example:
 *
 * int main(void) {
 *   if ( !iSTOCKFISH_Init() ) {
 *     return -1;
 *   }
 *
 *   iSTOCKFISH_GetBestMovement();
 *
 *   if ( DEBUG_MSGS ) vTraceMsg("Best Stockfish Movement: [%s]", gstStockfish.szBuffer);
 *
 *   vSTOCKFISH_End();
 *
 *   return 0;
 * }
 *
 * @return FALSE caso a API nao tenha sido inicializada
 * @return TRUE em caso de comunicacao bem sucedida.
 */
int bSTOCKFISH_GetBestMovement(void);

#endif
