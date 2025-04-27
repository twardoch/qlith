#pragma once

#include <litehtml.h>
#include <string>

/**
 * @brief Wrapper class for litehtml document creation
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
        master_css = css;
    }
    
    /**
     * @brief Get the master CSS
     * @return The master stylesheet content
     */
    const std::string& get_master_css() const
    {
        return master_css;
    }
    
private:
    std::string master_css;
}; 