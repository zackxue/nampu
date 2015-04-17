#ifndef _ERRORDEF_
#define _ERRORDEF_

#include "GeneralError.h"

#define SN_ERROR_DB_BEGIN				(-300)

//数据库打开失败
#define SN_ERROR_DB_OPEN_FAILED						(SN_ERROR_DB_BEGIN -1)

//数据库关闭失败
#define SN_ERROR_DB_CLOSE_FAILED					(SN_ERROR_DB_BEGIN - 2)

//开始事物失败
#define SN_ERROR_DB_BEGIN_TRANSACTION_ERROR			(SN_ERROR_DB_BEGIN - 3)

//回滚事物失败
#define SN_ERROR_DB_ROLLBACK_TRANSACTION_ERROR		(SN_ERROR_DB_BEGIN - 4)

//提交事物失败
#define SN_ERROR_DB_COMMIT_TRANSACTION_ERROR		(SN_ERROR_DB_BEGIN - 5)

//插入数据失败
#define SN_ERROR_DB_INSERT_ERROR					(SN_ERROR_DB_BEGIN - 6)

//删除数据失败
#define SN_ERROR_DB_DELETE_ERROR					(SN_ERROR_DB_BEGIN - 7)

//更新数据失败
#define SN_ERROR_DB_UPDATE_ERROR					(SN_ERROR_DB_BEGIN - 8)

//查询数据失败
#define SN_ERROR_DB_SELECT_ERROR					(SN_ERROR_DB_BEGIN - 9)

//查询条件错误
#define SN_ERROR_DB_SELECT_CONDITION_ERROR			(SN_ERROR_DB_BEGIN - 10)

//查询结果为空
#define SN_ERROR_DB_RECORDSET_EMPTY					(SN_ERROR_DB_BEGIN - 11)

#endif // _AVOBSERVER_H_
