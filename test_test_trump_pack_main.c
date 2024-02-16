#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NORMAL_CARD_COUNT 52 // 通常カードの枚数
#define EXTRA_CARD_COUNT 0   // 特殊カードの枚数
#define VERSION "TEST_005"   // バージョン

typedef struct /*トランプ1枚あたりに該当する情報を格納する。 id: 1 ~ 52 , mark: 1 ~ 4 ("♠", "♥", "♦", "♣") , number: 1 ~ 13, flag: 所持フラグとして利用可(プレイヤーIDを格納)*/
{
    int id;
    int mark;
    int number;
    int flag;
} trumps;

typedef struct /*プレイヤー1人あたりに1つ生成される構造体。id: プレイヤー通し番号(1 ~ ), name: プレイヤー名, point: 得点, hand_count: 手持ち札の枚数(転用可能) */
{
    int id;
    char name[50];
    int point;
    int hand_count;
} player_data;

trumps GLOBAL_TRUMPS_DATA_NORMAL[NORMAL_CARD_COUNT];
#if EXTRA_CARD_COUNT != 0
trumps GLOBAL_TRUMPS_DATA_EXTRA[EXTRA_CARD_COUNT];
#endif

void setup_GLOBAL_TRUMPS_DATA(void) // セットアップ. グローバルデータの初期書き込み.
{
    for (int i = 0; i < NORMAL_CARD_COUNT; i++)
    {
        GLOBAL_TRUMPS_DATA_NORMAL[i].id = i + 1;
        GLOBAL_TRUMPS_DATA_NORMAL[i].mark = (i / 13) + 1;
        GLOBAL_TRUMPS_DATA_NORMAL[i].number = (i % 13) + 1;
        GLOBAL_TRUMPS_DATA_NORMAL[i].flag = 0;
    }
#if EXTRA_CARD_COUNT != 0
    for (int i = 1; i < EXTRA_CARD_COUNT; i++)
    {
        GLOBAL_TRUMPS_DATA_EXTRA[i].id = i;
        GLOBAL_TRUMPS_DATA_EXTRA[i].flag = 0;
    }
#endif
}

void read_drawed_trumps(player_data player_data_to_read, trumps already_drawed_cards[],int* drawed_count) //グローバル情報にどのトランプが引かれているかをplayer_dataのタグとglobal変数から抽出する。また、intポインタのdrawed_countに何枚が検出されたのかを自動的に返す(ポインタ渡し)
{
    
    *drawed_count = 0;

    for(int i = 0; i < NORMAL_CARD_COUNT; i++) {
        if(GLOBAL_TRUMPS_DATA_NORMAL[i].flag == player_data_to_read.id) {
            (already_drawed_cards)[*drawed_count].id = GLOBAL_TRUMPS_DATA_NORMAL[i].id;
            (already_drawed_cards)[*drawed_count].mark = GLOBAL_TRUMPS_DATA_NORMAL[i].mark;
            (already_drawed_cards)[*drawed_count].number = GLOBAL_TRUMPS_DATA_NORMAL[i].number;
            (already_drawed_cards)[*drawed_count].flag = player_data_to_read.id;
            (*drawed_count)++;
        }
    }

}

trumps *draw_of_trumps(int number_of_draw, player_data player_data_of_drawer) // トランプを指定枚数重複しないように引き、結果を返却すると同時に全カードデータに書き戻す
{
    srand((unsigned int)time(NULL));
    int drawed_count = 0;
    trumps *drawed_cards = (trumps *)malloc(sizeof(trumps) * number_of_draw);
    trumps already_drawed_cards[NORMAL_CARD_COUNT];
    read_drawed_trumps(player_data_of_drawer,(&already_drawed_cards), &drawed_count);
    int *drawed_number = (int *)malloc(sizeof(int) * number_of_draw);
    
    int duplicate_flag = 0;
    int draw_num = 0, roupe_num = 0;
    
    

    while (roupe_num < number_of_draw)
    {
        duplicate_flag = 0;
        draw_num = (rand() % NORMAL_CARD_COUNT) + 1;
        for (int j = 0; j < roupe_num; j++)
        {
            if (draw_num == drawed_number[roupe_num])
            {
                duplicate_flag = 1;
            }
        }
        if (drawed_count != 0)
        {
            for (int j = 0; j < drawed_count; j++)
            {
                if (draw_num == already_drawed_cards[j].id)
                {
                    duplicate_flag = 1;
                }
            }
        }

        if (!duplicate_flag)
        {
            drawed_number[roupe_num] = draw_num;
            roupe_num++;
        }
    }

    for (int i = 0; i < number_of_draw; i++)
    {
        drawed_cards[i].id = drawed_number[i];
        drawed_cards[i].mark = ((drawed_number[i] - 1) / 13) + 1;
        drawed_cards[i].number = ((drawed_number[i] - 1) % 13) + 1;
        drawed_cards[i].flag = player_data_of_drawer.id;
    }

    for (int i = 0; i < number_of_draw; i++)
    {
        for (int j = 0; j < NORMAL_CARD_COUNT; j++)
        {
            if (drawed_cards[i].id == GLOBAL_TRUMPS_DATA_NORMAL[j].id)
            {
                GLOBAL_TRUMPS_DATA_NORMAL[j].flag = player_data_of_drawer.id;
            }
        }
    }

    free(drawed_number);

    return drawed_cards;
    free(drawed_cards);
}

int random_score_dice(int count_of_dice, int max_of_dice, int **dice_raw_return) // count_of_dice個の1 ~ max_of_diceまでの整数の合計を返し、dice_raw_returnにそれぞれの回の値を直接書き戻す
{

    int dice_return = 0;
    int temp_num = 0;
    srand((unsigned int)time(NULL));

    for (int i = 0; i < count_of_dice; i++)
    {
        temp_num = (rand() % max_of_dice) + 1;
        (*dice_raw_return)[i] = temp_num;
        dice_return += temp_num;
    }

    return dice_return;
}

int file_write(player_data *players_data, int count_of_player, char *file_name, char *file_mode) // ファイル書き込み。file_nameをfile_modeでファイルを開放、count_of_player人分のplayers_dataを書き込む
{
    time_t time_now = time(NULL);
    FILE *fp;
    char file_name_open[2048], file_name_date[128];
    int temp_num = 0, temp_pos = 0, score_compare[20] = {-1}, score_best_pos[20] = {0}, score_num_rank[20] = {0};

    struct tm *tm = localtime(&time_now);
    sprintf(file_name_date, "%4d/%2d/%2d %2d:%2d:%2d_PLAY-RECORD", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcat(file_name, file_name_open);
    fp = fopen(file_name_open, file_mode);
    if (fp == NULL)
    {
        return 1;
    }
    fprintf(fp, "%s\n", file_name_date);
    for (int i = 0; i < count_of_player; i++)
    {
        fprintf(fp, "%dP の スコア は %d \n", i + 1, players_data[i].point);

        temp_num = players_data[i].point;
        score_compare[i] = temp_num;
        score_best_pos[i] = i;
    }

    for (int i = 0; i < count_of_player; i++)
    {
        for (int j = i + 1; j < count_of_player; j++)
        {
            if (score_compare[i] > score_compare[j])
            {
                temp_num = score_compare[i];
                score_compare[i] = score_compare[j];
                score_compare[j] = temp_num;

                temp_pos = score_best_pos[i];
                score_best_pos[i] = score_best_pos[j];
                score_best_pos[j] = temp_pos;
            }
        }
    }
    temp_num = 1;
    for (int i = 0; i < 20; i++)
    {
        score_num_rank[i] = temp_num;
        if (score_compare[i] != score_compare[i + 1])
        {
            temp_num++;
        }
    }

    fprintf(fp, "並び替えた順位は、\n");
    for (int i = 0; i < count_of_player; i++)
    {
        fprintf(fp, "<%d位: %dP スコア: %d>\n\n", score_num_rank[i], score_best_pos[i] + 1, players_data[score_best_pos[i]].point);
    }

    fprintf(fp, "\n\n ---Thank you for playing !!!--- \n\n");

    fclose(fp);

    return 0;
}

player_data *player_data_gen(char **player_names, int player_count) // プレイヤーデータの初期作成
{
    player_data *generated_player_data = (player_data *)malloc(sizeof(player_data) * player_count);

    for (int i = 0; i < player_count; i++)
    {
        generated_player_data[i].id = i + 1;
        generated_player_data[i].hand_count = 0;
        generated_player_data[i].point = 0;
        sprintf(generated_player_data[i].name, "%s", player_names[i]);
    }

    return generated_player_data;
}

void show_score_per_player(int count_of_player, player_data *players_data) // count_of_player人のプレイヤーごとのスコアを表示
{
    for (int i = 0; i < count_of_player; i++)
    {
        printf("---score--- ");
    }
    printf("\n\n");

    for (int i = 0; i < count_of_player; i++)
    {
        printf("<PLAYER %02d> ", i + 1);
    }
    printf("\n\n");

    for (int i = 0; i < count_of_player; i++)
    {
        printf("<SCORE:%03d> ", players_data[i].point);
    }
    printf("\n\n");

    for (int i = 0; i < count_of_player; i++)
    {
        printf("---score--- ");
    }
    printf("\n");
}

void show_trumps_per_player(player_data player_data_to_show, trumps *trumps_data_to_show) // player_data構造体で渡されたプレイヤーの手札trumps_data_to_showを表示する
{
    int count_letter_top = player_data_to_show.hand_count / 2;
    char **number_letter_to_show = (char **)malloc(sizeof(char) * 2 * player_data_to_show.hand_count);
    const char *suits[] = {"♠", "♥", "♦", "♣"}; // 絵柄の配列

    if (player_data_to_show.hand_count % 2)
    {
        count_letter_top++;
    }

    for (int i = 0; i < player_data_to_show.hand_count; i++)
    {
        if (trumps_data_to_show[i].number == 1)
        {
            number_letter_to_show[i][0] = 'A';
        }
        else if (trumps_data_to_show[i].number == 10)
        {
            number_letter_to_show[i][0] = 'X';
        }
        else if (trumps_data_to_show[i].number == 11)
        {
            number_letter_to_show[i][0] = 'J';
        }
        else if (trumps_data_to_show[i].number == 12)
        {
            number_letter_to_show[i][0] = 'Q';
        }
        else if (trumps_data_to_show[i].number == 13)
        {
            number_letter_to_show[i][0] = 'K';
        }
        else
        {
            sprintf(number_letter_to_show[i], "%d", trumps_data_to_show[i].number);
        }
        number_letter_to_show[i][1] = '\0';
    }

    for (int i = 0; i < count_letter_top; i++)
    {
        printf("--PL%2d HAND-- ", player_data_to_show.id + 1);
    }
    printf("\n");

    for (int i = 0; i < player_data_to_show.hand_count; i++)
    {
        printf("______ ");
    }
    printf("\n");

    for (int i = 0; i < player_data_to_show.hand_count; i++)
    {
        printf("|%s  %s| ", number_letter_to_show[i], number_letter_to_show[i]);
    }
    printf("\n");

    for (int i = 0; i < player_data_to_show.hand_count; i++)
    {
        printf("|%s  %s| ", suits[trumps_data_to_show[i].mark - 1], suits[trumps_data_to_show[i].mark - 1]);
    }
    printf("\n");

    for (int i = 0; i < player_data_to_show.hand_count; i++)
    {
        printf("| ** | ");
    }
    printf("\n");

    for (int i = 0; i < player_data_to_show.hand_count; i++)
    {
        printf("|%s  %s| ", suits[trumps_data_to_show[i].mark - 1], suits[trumps_data_to_show[i].mark - 1]);
    }
    printf("\n");

    for (int i = 0; i < player_data_to_show.hand_count; i++)
    {
        printf("|%s  %s| ", number_letter_to_show[i], number_letter_to_show[i]);
    }
    printf("\n");

    for (int i = 0; i < player_data_to_show.hand_count; i++)
    {
        printf("￣￣￣ ");
    }
    printf("\n");

    for (int i = 0; i < count_letter_top; i++)
    {
        printf("--PL%2d HAND-- ", player_data_to_show.id + 1);
    }
    printf("\n");

    free(number_letter_to_show);
}

int main(void)
{
    setup_GLOBAL_TRUMPS_DATA(); // 初期設定を実行

    return 0;
}
