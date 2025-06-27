#ifndef LIBPL2_VERSION_
#define LIBPL2_VERSION_

#ifdef __cplusplus
extern "C" {
#endif

/*! @defgroup page_version Version Info
 *
 * @{
 */

/*! Store an emulator core or Parallel Launcher version */
typedef struct {
	unsigned short major;
	unsigned short minor;
	unsigned short patch;
} lpl2_version;

/*! Compares two @ref lpl2_version structs
 * @param[in] first A pointer to the first version
 * @param[in] second A pointer to the second version
 * @return Returns a negative number if the first version is earlier than the second version. \n
 * Returns zero if both versions are the same. \n
 * Returns a positive number if the first version is later than the second version.
 */
static inline int __attribute__((pure, warn_unused_result)) lpl2_compare_versions( const lpl2_version *first, const lpl2_version *second ) {
	int cmp = (int)first->major - (int)second->major;
	if( cmp != 0 ) return cmp;
	cmp = (int)first->minor - (int)second->minor;
	if( cmp != 0 ) return cmp;
	return (int)first->patch - (int)second->patch;
}

/*! @} */

#ifdef __cplusplus
}
#endif

#endif
