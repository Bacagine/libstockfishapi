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

#ifndef _WIN32
pid_t pidStockfish = -1;
int fdToStockfish[2];
int fdFromStockfish[2];
#endif

FILE *fpWrite = NULL;
FILE *fpRead = NULL;

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
  if ( !fpWrite || !fpRead ) return;
  fprintf(fpWrite, "uci\n");
  fflush(fpWrite);
  while ( fgets(gstStockfish.szBuffer, sizeof(gstStockfish.szBuffer), fpRead) ) {
    gstStockfish.szBuffer[strcspn(gstStockfish.szBuffer, "\r\n")] = '\0';
    if ( !strcmp(gstStockfish.szBuffer, "uciok") ) break;
  }
}

void vSTOCKFISH_Position(void) {
  STRUCT_MOVE_LIST *mv = NULL;
  char szPositions[2048] = "";
  if ( !fpWrite || !fpRead ) return;
  memset(szPositions, 0x00, sizeof(szPositions));
  for ( mv = gpstMoveList; mv; mv = mv->pstNext ) {
    strcat(szPositions, mv->stMovement.szMovement);
    if ( mv->pstNext )
      strcat(szPositions, " ");
  }
  fprintf(fpWrite, "position startpos moves %s\n", szPositions);
  fflush(fpWrite);
}

void vSTOCKFISH_GoMovetime(void) {
  if ( !fpWrite || !fpRead ) return;
  fprintf(fpWrite, "go movetime %s\n", gstStockfish.szMoveTime);
  fflush(fpWrite);
}

int bSTOCKFISH_IsReady(void) {
  int bRsl = 0;
  if ( !fpWrite || !fpRead ) return bRsl;
  fprintf(fpWrite, "isready\n");
  fflush(fpWrite);
  fgets(gstStockfish.szBuffer, sizeof(gstStockfish.szBuffer), fpRead);
  gstStockfish.szBuffer[strcspn(gstStockfish.szBuffer, "\r\n")] = '\0';
  if ( !strcmp(gstStockfish.szBuffer, "readyok") )
    bRsl = 1;
  return bRsl;
}

void vSTOCKFISH_Quit(void) {
  if ( !fpWrite || !fpRead ) return;
  fprintf(fpWrite, "quit\n");
  fflush(fpWrite);
}

int bSTOCKFISH_Init(const char *kpszStockfishPath, const char *kpszMoveTime) {
  memset(&gstStockfish, 0x00, sizeof(gstStockfish));

  if ( !kpszStockfishPath ) return 0;

  if ( !kpszMoveTime )
    snprintf(gstStockfish.szMoveTime, sizeof(gstStockfish.szMoveTime), "1000");
  else
    snprintf(gstStockfish.szMoveTime, sizeof(gstStockfish.szMoveTime), "%s", kpszMoveTime);

#ifndef _WIN32
  if ( pipe(fdToStockfish) < 0 ) {
    if ( DEBUG_MSGS ) vTraceMsg("pipe to stockfish failed: [%s]", strerror(errno));
    return FALSE;
  }

  if ( pipe(fdFromStockfish) < 0 ) {
    if ( DEBUG_MSGS ) vTraceMsg("pipe to stockfish failed: [%s]", strerror(errno));
    return FALSE;
  }

  pidStockfish = fork();
  if ( pidStockfish < 0 ) {
    if ( DEBUG_MSGS ) vTraceMsg("fork failed: [%s]", strerror(errno));
    return FALSE;
  }

  if ( pidStockfish == 0 ) {
    dup2(fdToStockfish[0], STDIN_FILENO);
    dup2(fdFromStockfish[1], STDOUT_FILENO);

    close(fdToStockfish[0]);
    close(fdToStockfish[1]);
    close(fdFromStockfish[0]);
    close(fdFromStockfish[1]);

    execlp(kpszStockfishPath, kpszStockfishPath, NULL);
    if ( DEBUG_MSGS ) vTraceMsg("exec failed: [%s]", strerror(errno));
    exit(EXIT_FAILURE);
  }
  else {
    close(fdToStockfish[0]);
    close(fdFromStockfish[1]);

    fpWrite = fdopen(fdToStockfish[1], "w");
    fpRead = fdopen(fdFromStockfish[0], "r");

    if ( !fpWrite || !fpRead ) {
      if ( DEBUG_MSGS ) vTraceMsg("fdopen failed: [%s]", strerror(errno));
      return FALSE;
    }
  }
#endif
  gstStockfish.bStockfishOn = 1;

  return 1;
}

int bSTOCKFISH_IsStarted(void) {
  return gstStockfish.bStockfishOn;
}

void vSTOCKFISH_End(void) {
  vSTOCKFISH_Quit();

  if ( fpWrite ) {
    fclose(fpWrite);
    fpWrite = NULL;
  }

  if ( fpRead ) {
    fclose(fpRead);
    fpRead = NULL;
  }
#ifndef _WIN32
  if ( pidStockfish > 0 ) {
    int iStatus = 0;
    waitpid(pidStockfish, &iStatus, 0);
    pidStockfish = -1;
  }
#endif
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
  while ( fgets(gstStockfish.szBuffer, sizeof(gstStockfish.szBuffer), fpRead) ) {
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
