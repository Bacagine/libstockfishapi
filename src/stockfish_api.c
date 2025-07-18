/**
 * @file stockfish_api.c
 *
 * @brief Integrates with stockfish
 *
 * @author Gustavo Bacagine <gustavo.bacagine@protonmail.com>
 *
 * @date Jul 2025
 */

#include "stockfish_api.h"

STRUCT_MOVE_LIST *gpstMoveList = NULL;
STRUCT_STOCKFISH gstStockfish;

pid_t pidStockfish;
int fdToStockfish[2];
int fdFromStockfish[2];

int bCreateMoveList(void) {
  gpstMoveList = NULL;
  return 1;
}

void vDestroyMoveList(void) {
  PSTRUCT_MOVE_LIST pstWrkMoveList = gpstMoveList;
  while ( pstWrkMoveList ) {
    PSTRUCT_MOVE_LIST pstNext = pstWrkMoveList->pstNext;
    free(pstWrkMoveList);
    pstWrkMoveList = pstNext;
  }
  gpstMoveList = NULL;
}

int bMoveListIsEmpty(void) {
  return (gpstMoveList == NULL);
}

int bAddMoveToList(STRUCT_MOVEMENT *pstMovement) {
  const char kachRows[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
  PSTRUCT_MOVE_LIST pstWrkMoveList = (STRUCT_MOVE_LIST *) calloc(1, sizeof(STRUCT_MOVE_LIST));

  if ( !pstWrkMoveList ) return 0;

  pstWrkMoveList->stMovement = *pstMovement;
  snprintf(
    pstWrkMoveList->stMovement.szMovement,
    sizeof(pstWrkMoveList->stMovement.szMovement),
    "%c%d%c%d",
    kachRows[pstMovement->iStartY],
    pstMovement->iStartX+1,
    kachRows[pstMovement->iEndY],
    pstMovement->iEndX+1
  );

  if ( !gpstMoveList ) {
    gpstMoveList = pstWrkMoveList;
  }
  else {
    PSTRUCT_MOVE_LIST pstLast = gpstMoveList;
    while ( pstLast->pstNext )
      pstLast = pstLast->pstNext;
    pstLast->pstNext = pstWrkMoveList;
  }

  return 1;
}

void vSTOCKFISH_UCI(void) {
  fprintf(stdin, "uci\n");
  fflush(stdin);
}

void vSTOCKFISH_Position(void) {
  STRUCT_MOVE_LIST *mv = NULL;
  char szPositions[2048] = "";
  memset(szPositions, 0x00, sizeof(szPositions));
  for ( mv = gpstMoveList; mv; mv = mv->pstNext ) {
    strcat(szPositions, mv->stMovement.szMovement);
    if ( mv->pstNext )
      strcat(szPositions, " ");
  }
  fprintf(stdin, "position startpos moves %s\n", szPositions);
  fflush(stdin);
}

void vSTOCKFISH_GoMovetime(void) {
  fprintf(stdin, "go movetime %s\n", gstStockfish.szMoveTime);
  fflush(stdin);
}

int bSTOCKFISH_IsReady(void) {
  int bRsl = 0;
  fprintf(stdin, "isready\n");
  fflush(stdin);
  fgets(gstStockfish.szBuffer, sizeof(gstStockfish.szBuffer), stdin);
  gstStockfish.szBuffer[strcspn(gstStockfish.szBuffer, "\r\n")] = '\0';
  if ( !strcmp(gstStockfish.szBuffer, "readyok") )
    bRsl = 1;
  return bRsl;
}

void vSTOCKFISH_Quit(void) {
  fprintf(stdin, "quit\n");
  fflush(stdin);
}

int bSTOCKFISH_Init(const char *kpszStockfishPath, const char *kpszMoveTime) {
  memset(&gstStockfish, 0x00, sizeof(gstStockfish));

  if ( !kpszStockfishPath ) return 0;

  if ( !kpszMoveTime )
    snprintf(gstStockfish.szMoveTime, sizeof(gstStockfish.szMoveTime), "1000");
  else
    snprintf(gstStockfish.szMoveTime, sizeof(gstStockfish.szMoveTime), "%s", kpszMoveTime);

  if ( pipe(fdToStockfish) < 0 ) {
    perror("pipe to stockfish failed");
    return 0;
  }

  if ( pipe(fdFromStockfish) < 0 ) {
    perror("pipe to stockfish failed");
    return 0;
  }

  pidStockfish = fork();
  if ( pidStockfish < 0 ) {
    perror("fork failed");
    return 0;
  }
  /* bacagine - 17/07/2025 - TODO: Continuar o desenvolvimento da API a partir desse ponto */

  return 1;
}

int bSTOCKFISH_IsStarted(void) {
  return !(pidStockfish < 0);
}

void vSTOCKFISH_End(void) {
  vSTOCKFISH_Quit();
  // pclose(stdin);
  memset(&gstStockfish, 0x00, sizeof(gstStockfish));
}

int bSTOCKFISH_GetBestMovement(void) {
  char *pTok = NULL;
  char szBuffer[2048] = "";

  memset(szBuffer, 0x00, sizeof(szBuffer));

  if ( !bSTOCKFISH_IsStarted() ) {
    return 0;
  }

  vSTOCKFISH_UCI();
  if  ( !bSTOCKFISH_IsReady() ) {
    return 0;
  }

  vSTOCKFISH_Position();
  vSTOCKFISH_GoMovetime();
  while ( fgets(gstStockfish.szBuffer, sizeof(gstStockfish.szBuffer), stdin) ) {
    if ( strstr(gstStockfish.szBuffer, "bestmove") ) {
      break;
    }
  }

  snprintf(szBuffer, sizeof(szBuffer), "%s", gstStockfish.szBuffer);
  pTok = strtok(szBuffer, " ");
  pTok = strtok(NULL, " ");
  if ( pTok )
    snprintf(gstStockfish.szBestMove, sizeof(gstStockfish.szBestMove), "%s", pTok);
  else
    gstStockfish.szBestMove[0] = '\0';

  return 1;
}
