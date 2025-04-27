#pragma once

#include <litehtml.h>
#include <string>

/**
 * @brief Wrapper class for litehtml context
 */
class litehtml_context
{
public:
    /**
     * @brief Constructor
     */
    litehtml_context() = default;
    
    /**
     * @brief Load the master stylesheet
     * @param css CSS stylesheet content
     */
    void load_master_stylesheet(const char* css)
    {
        m_context.load_master_stylesheet(css);
    }
    
    /**
     * @brief Get the litehtml context object
     * @return Reference to the internal litehtml context
     */
    litehtml::context& get_context()
    {
        return m_context;
    }
    
    /**
     * @brief Get a pointer to the litehtml context
     * @return Pointer to the internal litehtml context
     */
    litehtml::context* get_context_ptr()
    {
        return &m_context;
    }
    
private:
    litehtml::context m_context;
}; 