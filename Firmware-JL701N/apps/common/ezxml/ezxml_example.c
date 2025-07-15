
#include "ezxml.h"

#define MULTI_LINE_STRING(a) #a

/* --------------------------------------------------------------------------*/
/**
 * Given the following example XML document:
 *
 *  <?xml version="1.0"?>
 *  <formula1>
 *      <team name="McLaren">
 *          <driver>
 *              <name>Kimi Raikkonen</name>
 *              <points>112</points>
 *          </driver>
 *          <driver>
 *              <name>Juan Pablo Montoya</name>
 *              <points>60</points>
 *          </driver>
 *      </team>
 *  </formula1>
 */
/* ----------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------*/
/**
 * @brief ezxml_example_1
 */
/* ----------------------------------------------------------------------------*/
#include "fs.h"
#define XML_FILE_NAME 		SDFILE_RES_ROOT_PATH"ezxml.xml"
void ezxml_example_1(void)
{
    printf("%s", __FUNCTION__);

    ezxml_t f1 = ezxml_parse_file(XML_FILE_NAME), team, driver;
    const char *teamname;

    for (team = ezxml_child(f1, "team"); team; team = team->next) {
        teamname = ezxml_attr(team, "name");
        for (driver = ezxml_child(team, "driver"); driver; driver = driver->next) {
            printf("%s, %s: %s\n", ezxml_child(driver, "name")->txt, teamname,
                   ezxml_child(driver, "points")->txt);
        }
    }
    ezxml_free(f1);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief ezxml_example_2
 */
/* ----------------------------------------------------------------------------*/

// *INDENT-OFF*
static const char *xml_msg= MULTI_LINE_STRING(
<?xml version='1.0' encoding='utf-8' standalone='yes' ?>
<MAP-msg-listing version="1.0">
    <msg handle="04000000000011FF" subject="message 1 context" datetime="20220405T101950" sender_name="" sender_addressing="10000" recipient_name="" recipient_addressing="" type="SMS_GSM" size="237" text="yes" reception_status="complete" attachment_size="0" priority="no" read="yes" sent="no" protected="no" />
    <msg handle="04000000000011EA" subject="MESSAGE 2 context" datetime="20220307T102547" sender_name="" sender_addressing="10001" recipient_name="" recipient_addressing="" type="SMS_GSM" size="45" text="yes" reception_status="complete" attachment_size="0" priority="no" read="yes" sent="no" protected="no" />
    <msg handle="04000000000011E4" subject="MESSAGE three context" datetime="20220301T095749" sender_name="" sender_addressing="10000" recipient_name="" recipient_addressing="" type="SMS_GSM" size="189" text="yes" reception_status="complete" attachment_size="0" priority="no" read="yes" sent="no" protected="no" />
    <msg handle="04000000000011E3" subject="message destory context" datetime="20220_GSM" size="237" text="yes" />
</MAP-msg-listing>
);
// *INDENT-ON*

void ezxml_example_2(void)
{
    ezxml_t f1, msg;

    printf("%s", __FUNCTION__);

    char *buf = malloc(strlen(xml_msg));
    memcpy(buf, xml_msg, strlen(xml_msg));
    f1 = ezxml_parse_str(buf, strlen(xml_msg));

    msg = ezxml_child(f1, "msg");

    for (msg = ezxml_child(f1, "msg"); msg; msg = msg->next) {
        printf("handle:%s, subject:%s, datetime:%s, size:%s", ezxml_attr(msg, "handle"), ezxml_attr(msg, "subject"), ezxml_attr(msg, "datetime"), ezxml_attr(msg, "size"));
    }

    ezxml_free(f1);
    free(buf);
}



/* --------------------------------------------------------------------------*/
/**
 * @brief ezxml_example_3
 */
/* ----------------------------------------------------------------------------*/
// *INDENT-OFF*
static const char *xml_str = MULTI_LINE_STRING(
    <?xml version="1.0"?>
    <formula1>
      <team name="McLaren">
        <driver>
          <name>Kimi Raikkonen</name>
          <points>112</points>
        </driver>
        <driver>
          <name>Juan Pablo Montoya</name>
          <points>60</points>
        </driver>
      </team>
    </formula1>
);
// *INDENT-ON*

void ezxml_example_3(void)
{
    ezxml_t f1, team, driver;
    const char *teamname;

    printf("%s", __FUNCTION__);

    char *buf = malloc(strlen(xml_str));
    memcpy(buf, xml_str, strlen(xml_str));
    f1 = ezxml_parse_str(buf, strlen(xml_str));

    for (team = ezxml_child(f1, "team"); team; team = team->next) {
        teamname = ezxml_attr(team, "name");
        for (driver = ezxml_child(team, "driver"); driver; driver = driver->next) {
            printf("%s, %s: %s\n", ezxml_child(driver, "name")->txt, teamname,
                   ezxml_child(driver, "points")->txt);
        }
    }
    ezxml_free(f1);
    free(buf);
}

