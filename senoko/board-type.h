#ifndef __SENOKO_BOARD_TYPE_H__
#define __SENOKO_BOARD_TYPE_H__

enum board_type {
  senoko_unknown,
  senoko_full,
  senoko_passthru,
};

enum board_type boardType(void);
void boardTypeInit(void);

#endif /* __SENOKO_BOARD_TYPE_H__ */
